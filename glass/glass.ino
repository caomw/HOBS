#include <SoftwareSerial.h>
#include <IRremote.h>
#include <assert.h>
#include <TimerOne.h>

// define all the states here
#define IDLE 0
#define INIT 1
#define WAIT 2
#define CONFIRM 3
#define VERIFY 4
#define CONNECTED 5
#define SOFTPOT_THREASHOLD 950
#define DELAY_IN_WAIT 1000000
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
int ledPin = 8;
int state = 0;
int softpotReading = 0;
struct XBeeMessage XBeeMsgArr[20];
int XBeeMsgPointer;
volatile int selectedXBee;
unsigned long start_time;
unsigned long end_time;


const char segmentDeliminater = ':';

void setup()
{
  Serial.begin(9600);
  state = IDLE;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBee.begin(9600);
  XBeeMsgPointer = 0;
}

void checkForReplies(int num, int *state) {
  // check for the number of replies and act accordingly
  Serial.println("FUCK");
  Serial.println(num);
  if (num == 0) {
    Serial.println("No Msg received");
    // No packet received, or damaged packet received
    *state = INIT;
  }
  else if (num == 1) {
    // only one responded, cool, just go to confirm stage
    *state = CONFIRM;
    selectedXBee = XBeeMsgPointer - 1;
    Serial.print("[CONFIRM] entering with id=");
    Serial.println(XBeeMsgArr[selectedXBee].deviceId);
  }
  else {
    Serial.print("[VERIFYING] entering with Number of devices");
    Serial.print(XBeeMsgPointer);
    *state = VERIFY;
  }
}

void loop() {
  switch(state) {
  case IDLE:
    softpotReading = analogRead(softpotPin);
    Serial.print("Softpot Reading: ");
    Serial.println(softpotReading);
    delay(20);
    if (softpotReading < SOFTPOT_THREASHOLD) {
      Serial.println("[INIT] Entering");      
      state = INIT;
    }
    selectedXBee = -1;
    break;
  case INIT:
    Serial.println("[INIT] Sending IR");
    session_id = random(0xFFFF);
    // irsend.sendSony(session_id, 16);
    XBee.print(session_id);   

    // at the same time listening to any response
    state = WAIT;
    Serial.println("[WAIT] entering state");
    XBeeMsgPointer = 0;
    start_time = millis();
    break;
  case WAIT:
    // When entering this state, wait for 1 sec to determine how many signals have been received
    end_time = millis();
    if (end_time - start_time > DELAY_IN_WAIT/1000) {
      // soft timer expires
      Serial.println(XBeeMsgPointer);
      // check for the number of replies and act accordingly
      Serial.println("FUCK");

      if (XBeeMsgPointer == 0) {
	Serial.println("No Msg received");
      }
      else if (XBeeMsgPointer == 1) {
	selectedXBee = XBeeMsgPointer - 1;
      	Serial.print("[CONFIRM] entering with id=");
      	Serial.println(XBeeMsgArr[selectedXBee].deviceId);
	state = CONFIRM;
      }
      
      /* 	Serial.println("No Msg received"); */
      /* 	// No packet received, or damaged packet received */
      /* 	state = INIT; */
      /* } */
      /* else if (XBeeMsgPointer == 1) { */
      /* 	// only one responded, cool, just go to confirm stage */
      /* 	state = CONFIRM; */
      /* 	selectedXBee = XBeeMsgPointer - 1; */
      /* 	Serial.print("[CONFIRM] entering with id="); */
      /* 	Serial.println(XBeeMsgArr[selectedXBee].deviceId); */
      /* } */
      /* else { */
      /* 	Serial.print("[VERIFYING] entering with Number of devices"); */
      /* 	Serial.print(XBeeMsgPointer); */
      /* 	state = VERIFY; */
      /* } */
    }
    if(XBee.available()) {
      Serial.println("[WAIT] in state, adding new data");
      // delay for the complete of transmission
      delay(10);
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
      Serial.print("[WAIT] Get Packet:");
      Serial.print(packet);
      
      // a weak design, you should be cautious when packets are corrupted
      // do a checksum
      char *pos = strchr (packet, ':');
      if (pos == NULL || pos == packet) {
	Serial.println("Corrupted packet");
      }
      else {
	str = strtok_r(p, ":", &p);
	strcpy(id, str);
	str = strtok_r(p, ":", &p);
	strcpy(msg, str);

	// assert( (str = strtok_r(p, ":", &p)) != NULL);
	/* // for debugging */
	/* Serial.println("[WAIT] Get XBee reading:"); */
	/* Serial.print("id:"); */
	/* Serial.print(id); */
	/* Serial.print(" msg:"); */
	/* Serial.println(msg); */
	
	for(i = 0; i <= XBeeMsgPointer; i++) {
	  if (XBeeMsgArr[i].deviceId == atoi(id))
	    break;
	}
	if (i > XBeeMsgPointer) {
	  Serial.print("Found new id:"); 
	  Serial.println(id);
	  // not found, insert it
	  XBeeMsgArr[XBeeMsgPointer].deviceId = atoi(id);
	  XBeeMsgArr[XBeeMsgPointer].msg = atoi(msg);
	  XBeeMsgPointer++;
	  break;
	}
      }
    }
    break;
  case CONFIRM:
    // send out the signal to THE selected XBee
    assert(selectedXBee == -1);
    XBee.print(XBeeMsgArr[selectedXBee].deviceId);
    XBee.print(":");
    XBee.println("Confirmation");
    Serial.print(XBeeMsgArr[selectedXBee].deviceId);
    Serial.print(":");
    Serial.println("Confirmation");
    state = CONNECTED;
    break;
  case VERIFY:
    // when there are multiple targets who have responded
    // one of the LED should be on at this case

    // digitalWrite(ledPin, HIGH);
    // then based on the gesture, we start to rotate them 
    state = CONNECTED;
    break;
  case CONNECTED:

    // temporarily for debugging 
    delay(3000);
    state = INIT;
    // start Gesture recognition and communication
    break;
  default:
    break;
  }
}
