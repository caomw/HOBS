#include <SoftwareSerial.h>
#include <IRremote.h>
#include <AikoEvents.h>

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
// An IR LED must be connected to Arduino PWM pin 3.

IRsend irsend;
int softpotPin = A0; //analog pin 0
int sendState = 0;
int ledPin = 13;
int state = 0;
int softpotReading = 0;
void setup()
{
  Serial.begin(9600);
  state = IDLE;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
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
      break;
    case INIT:
      session_id = random(0xFFFF);
      irsend.sendSony(session_id, 16);
      state = WAIT;
      break;
    case WAIT:
      // at the same time listening to any response
      if(XBee.available()) {
        digitalWrite(ledPin, HIGH);
        Serial.write(XBee.read());
        XBee.write("roger");
      }
      break;
    case CONFIRM:
      break;
    case VERIFY:
      break;
    case CONNECTED:
      break;
    default:
      break;
  }
//   Serial.println(readCapacitivePin(7));
//   delay (300); 
//  irsend.sendSony(0xa90, 12);
//  delay(100);
//  Serial.print("sending a90: ");
//  Serial.println(count++);
//
//  irsend.sendSony(0xa10, 12);
//  delay(100);
//  Serial.print("sending a10: ");
//  Serial.println(count++);
  
//  if (readCapacitivePin(7) == 3 && sendState == 0) {
//    irsend.sendSony(0xa90, 12); // Sony TV power code
//    Serial.print("sending ");
//    Serial.println(count++);
//    sendState = 1;
//  }
//  else if (readCapacitivePin(7) == 2 && sendState == 1) {
//    sendState = 0;
//  }

//  if (Serial.read() != -1) {
//    for (int i = 0; i < 3; i++) {
//      irsend.sendSony(0xa90, 12); // Sony TV power code
//      delay(40);
//    }
//  }
}
