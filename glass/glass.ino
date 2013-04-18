#include <SoftwareSerial.h>
#include <IRremote.h>

#include "utils.h"

SoftwareSerial XBee(2, 4); // RX, TX

// define all the states here
#define IDLE 0
#define INIT 1
#define WAIT 2
#define CONFIRM 3
#define VERIFY 4
#define CONNECTED 5
#define SOFTPOT_THREASHOLD 950
#define DELAY_IN_WAIT 1000000
unsigned long session_id = 0xA90;

// An IR LED must be connected to Arduino PWM pin 3.
IRsend irsend;
int softpotPin = A0; //analog pin 0
int sendState = 0;
int ledPin = 8;
int state = 0;
int softpotReading = 0;
struct XBeePacket XBeePacketArr[20];
volatile int XBeePacketCounter;
volatile int selectedXBee;
unsigned long start_time;
unsigned long end_time;

void setup()
{
  Serial.begin(9600);
  state = IDLE;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBee.begin(9600);
  XBeePacketCounter = 0;
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
    XBee.print(session_id, HEX);  

    // at the same time listening to any response
    state = WAIT;
    Serial.println("[WAIT] entering state");
    XBeePacketCounter = 0;
    start_time = millis();
    break;
  case WAIT:
    // When entering this state, wait for 1 sec to determine how many signals have been received
    end_time = millis();
    // soft timer expires
    if (end_time - start_time > DELAY_IN_WAIT/1000) {
      // check for the number of replies and act accordingly
      Serial.println(XBeePacketCounter);

      if (XBeePacketCounter == 0) {
	Serial.println("No Msg received");
	state = IDLE;
      }
      else if (XBeePacketCounter == 1) {
	selectedXBee = XBeePacketCounter - 1;
      	Serial.print("[CONFIRM] entering with selected: ");
	Serial.print(selectedXBee);
	Serial.print(" deviceId:");
	Serial.println(XBeePacketArr[selectedXBee].id);
	// send out the selected message
	sendXBeePacketFromRaw(&XBee, XBeePacketArr[selectedXBee].id, "c", XBeePacketArr[selectedXBee].data);
	state = CONFIRM;
      }
    }
    if(XBee.available()) {
      int i = 0;
      // delay for the complete of transmission
      delay(10);
      Serial.println("[WAIT] Reading Packet");
      struct XBeePacket p = readXBeePacket(&XBee);
      printXBeePacket(p);

      // now add this packet to packet array if id is new
      for (i = 0; i < XBeePacketCounter; i++) {
	if ( atoi(p.id) == atoi(XBeePacketArr[i].id) ) {
	  break;
	}
      }
      if (i == XBeePacketCounter) {
	// not found, add to the array
	XBeePacketArr[i] =  p;
	XBeePacketCounter++;
      }
    }
    break;
  case CONFIRM:
    // send out the signal to THE selected XBee
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
    delay(10000);
    state = IDLE;
    
    // start Gesture recognition and communication

    break;
  default:
    break;
  }
}


