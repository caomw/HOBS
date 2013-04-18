#include <SoftwareSerial.h>
#include <IRremote.h>
#include "utils.h"

// Trying to elucidate the way communication works first
// current design enforces a simplest design on the receiver side
// no scheduling, purely acknowledge everything it has received with a random delay
#define IDLE 0
#define PENDING 1
#define CONNECTED 2
#define DELAY_IN_WAIT 1000000

SoftwareSerial XBee(2, 3); // RX, TX
int ledPin = 13;
int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;
int deviceStatus = 0;
int pendingTimer;
char deviceId[2] = "02";
char XBeeInString[50];
int state = IDLE;
unsigned long randomDelay = 0;
unsigned long start_time;
unsigned long end_time;

#define statusOff 0
#define statusPending 1
#define statusOn 2

// const unsigned long signalInit = 0xA90;
const unsigned long signalInit = 0xC1AA09F6;
const unsigned long signalVerify = 0xAF0;
const unsigned long signalConfirm = 0xA70;

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);   
  Serial.println("system begins!");
  // set the data rate for the SoftwareSerial port
  XBee.begin(9600);
  irrecv.enableIRIn(); // Start the receiver

  randomSeed(analogRead(5));
}

void loop()
{
  switch(state) {
  case IDLE:
    // purely listening and then react by response
    // for now test use random
    // if(irrecv.decode(&results)) {
    
    if(XBee.available()) {
      delay(5);
      char packet[50];
      int len = readXBeeString(packet);
      
      Serial.print("\nPacket received: ");
      Serial.print(packet);
      Serial.print("  Packet len: ");
      Serial.println(len);

      randomDelay = random(1000);
      delay(randomDelay);

      // send back acknowledge packet
      struct XBeePacket p;
      string_copy(p.id, deviceId, 0, 1);
      string_copy(p.type, "a", 0, 0);
      string_copy(p.data, packet, 0, 3);
      string_copy(p.cksum, "0", 0, 0);
      printXBeePacket(p);
      sendXBeePacket(&XBee, p);
      start_time = millis();
      state = PENDING;
    }
    break;
  case PENDING:
    end_time = millis();
    // soft timer expires
    if (end_time - start_time > DELAY_IN_WAIT/1000) {
      Serial.println("[PENDING] Timer expires, back to IDLE");
      // timeout, return to IDLE
      state = IDLE;
    }
    if(XBee.available()) {
      // delay for the complete of transmission
      delay(10);
      Serial.println("[WAIT] Reading Packet");
      struct XBeePacket p = readXBeePacket(&XBee);
      printXBeePacket(p);

      if (atoi(p.id) == atoi(deviceId) && p.type[0] == 'c') {
      	// have been confirmed
      	Serial.println("[CONNECTED] entering state");

      	// turn on the ligth to indicate
      	digitalWrite(ledPin, HIGH);

      	state = CONNECTED;
      }
            // verifying this selection
      else if (atoi(p.id) == atoi(deviceId) && p.type[0] == 'v') {
      	// have been confirmed
      	Serial.println("[WAIT] being verified");

      	// may flash the light to indicate this
      	
      	state = PENDING;
      }      
      // verifying this selection
      else if (atoi(p.id) != atoi(deviceId) && p.type[0] == 'v') {
      	// have been confirmed
      	Serial.println("[WAIT] verifying others");
      	state = PENDING;
      }      
      else {
      	Serial.println("[IDLE] id not equal");
      	state = IDLE;
      }
    }
    break;

  case CONNECTED:

    // temporarily for debugging 
    delay(10000);
    state = IDLE;
    break;

  default:
    break;
  }
  
}

void sendMsg(String msg) {
  Serial.print(deviceId);
  Serial.print(": ");
  Serial.print(msg);
  Serial.println();

  XBee.print(deviceId);
  XBee.print(": ");
  XBee.print(msg);
  XBee.println();

}

int readXBeeString (char *strArray) {
  int i = 0;
  if(!XBee.available()) {
    return -1;
  }
  while (XBee.available()) {
    strArray[i] = XBee.read();
    i++;
  }
  strArray[i] = '\0';
  return i;
  
  Serial.print("read XBee: ");
  Serial.println(strArray);
  // sendMsg(XBeeInString);
}
