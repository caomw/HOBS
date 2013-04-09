#include <SoftwareSerial.h>
#include <IRremote.h>
#include <assert.h>

// define all the states here
#define IDLE 0
#define INIT 1
#define WAIT 2
#define CONFIRM 3
#define VERIFY 4
#define CONNECTED 5
#define SOFTPOT_THREASHOLD 950

SoftwareSerial XBee(2, 4); // RX, TX
unsigned long session_id = 0xA90;

struct XBeeMessage {
  int deviceId;
  unsigned long msg;
};

// An IR LED must be connected to Arduino PWM pin 3.
IRsend irsend;
int softpotPin = A0; //analog pin 0
int sendState = 0;
int ledPin = 13;
int state = 0;
int softpotReading = 0;
unsigned long start_time;
unsigned long new_time;
struct XBeeMessage XBeeMsgArr[20];
int XBeeMsgPointer;
int selectedXBee;
  
void setup()
{
  Serial.begin(9600);
  state = INIT;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBee.begin(9600);
  XBeeMsgPointer = 0;
}

void loop() {
  switch(state) {
  case IDLE:
    softpotReading = analogRead(softpotPin);
    Serial.print("Softpot Reading: ");
    Serial.println(softpotReading);
    delay(20);
    if (softpotReading < SOFTPOT_THREASHOLD) {
      state = INIT;
    }
    selectedXBee = -1;
    break;
  case INIT:
    Serial.println("[INIT] Sending IR");
    session_id = random(0xFFFF);
    irsend.sendSony(session_id, 16);
    // at the same time listening to any response
    if(XBee.available()) {
      state = WAIT;
      start_time = millis();
      Serial.print("[WAIT] entering state, time=");
      Serial.println(start_time);
      XBeeMsgPointer = 0;
    }
    // this delay is purely for rate limiting now
    delay(30);
    break;
  case WAIT:
    // When entering this state, wait for 1 sec to determine how many signals have been received
    new_time = millis() - start_time;
    if (new_time > 1000) {
      Serial.print("[WAIT] One sec expires, client count:  ");      
      Serial.println(XBeeMsgPointer);
      if (XBeeMsgPointer == 0) {
	// 
      }
      else if (XBeeMsgPointer == 1) {
	// only one responded, cool, just go to confirm stage
	Serial.println("[CONFIRM] entering");
	state = CONFIRM;
	selectedXBee = XBeeMsgPointer - 1;
      }
      else {
	state = VERIFY;
      }
    }
    if(XBee.available()) {
      char packet[50];
      char id[10];
      char msg[40];
      int i = 0;
      char *str;
      char *p = packet;
      
      // obtain the packet
      while (XBee.available()) {
	packet[i++] = (char) XBee.read();	
	// Serial.print( (char) ( XBee.read()) );
      }
      packet[i] = '\0';

      // a weak design, you should be cautious when packets are corrupted
      str = strtok_r(p, ":", &p);
      strcpy(id, str);
      str = strtok_r(p, ":", &p);
      strcpy(msg, str);

      // assert( (str = strtok_r(p, ":", &p)) != NULL);
      // for debugging
      Serial.println("[WAIT] Get XBee reading:");
      Serial.print("id:");
      Serial.print(id);
      Serial.print(" msg:");
      Serial.println(msg);
      
      for(i = 0; i <= XBeeMsgPointer; i++) {
	if (XBeeMsgArr[i].deviceId == atoi(id))
	  break;
      }
      if (i > XBeeMsgPointer) {
	// not found, insert it
	XBeeMsgArr[XBeeMsgPointer].deviceId = atoi(id);
	XBeeMsgArr[XBeeMsgPointer].msg = atoi(msg);
	XBeeMsgPointer++;
	break;
      }      
    }
    break;
  case CONFIRM:
    // send out the signal to THE selected XBee
    assert(selectedXBee == -1);
    XBee.print(XBeeMsgArr[selectedXBee].deviceId);
    XBee.print(": ");
    XBee.println("Confirmation");
    state = CONNECTED;
    break;
  case VERIFY:
    // when there are multiple targets who have responded
    // one of the LED should be on at this case
    digitalWrite(ledPin, HIGH);
    // then based on the gesture, we start to rotate them 
  
    break;
  case CONNECTED:
    // start Gesture recognition and communication
    break;
  default:
    break;
  }
}
