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
  state = IDLE;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBeeMsgPointer = 0;
}

void loop() {
  switch(state) {
  case IDLE:
    softpotReading = analogRead(softpotPin);
    Serial.print("Softpot Reading: ");
    Serial.println(softpotReading);    
    if (softpotReading < SOFTPOT_THREASHOLD) {
      state = INIT;
    }
    selectedXBee = -1;
    break;
  case INIT:
    session_id = random(0xFFFF);
    irsend.sendSony(session_id, 16);
    // at the same time listening to any response
    if(XBee.available()) {
      state = WAIT;
      start_time = millis();
      XBeeMsgPointer = 0;
    }
    break;
  case WAIT:
    // When entering this state, wait for 1 sec to determine how many signals have been received
    new_time = millis() - start_time;
    if (new_time > 1000) {
      // enter to a new state based on counts
      if (XBeeMsgPointer == 1) {
	// only one responded, cool, just go to confirm stage
	state = CONFIRM;
	selectedXBee = XBeeMsgPointer - 1;
      }
      else {
	state = VERIFY;
      }
    }      
    if(XBee.available()) {
      switch(XBee.read()) {

      // here need debugging to see what is the message like
      case 0: // "success"
	// XBeeMsgArr[XBeeMsgPointer].deviceId = id;
	// XBeeMsgArr[XBeeMsgPointer].msg = msg;
	XBeeMsgPointer++;
	break;
      default:
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
