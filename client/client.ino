/*
  client.ino

  This is a general framework to fit in any clients that can be remotely controlled with XBee. Each clients will add additional function of parsing certain commands, while the major application takes charge of external communication.

  Examples of clients include, for now, lamps and laptops.

  Lamps are simple, dump clients that only support on/off.
  Laptops now are viewed as video players, thus supporting play/pause, volume adjustment, and brightness adjustment, etc.


  Created 07/17/2013
  By benzh@eecs.berkeley.edu

*/

#include <SoftwareSerial.h>
#include <IRremote.h>
#include <string.h>
#define DEBUG 1

#include "utils.h"


// In this simplified design, there is no need to save states on client for feedback
// so I have deleted all state variables like IDLE, PENDING, CONNECTED, etc.

SoftwareSerial XBee(2, 3); // RX, TX
int ledStatePin = 13;
int ledSignalPin = 11;
int controlledPin = 12;

int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;

char deviceId[3] = "??";
char XBeeInString[50];

unsigned int ledStateInterval = 500;
unsigned long randomDelay = 0;
unsigned long start_time;
unsigned long end_time;
unsigned long toggle_time;
unsigned long signal_time;

#define statusOff 0
#define statusPending 1
#define statusOn 2

void setup()  
{
  Serial.begin(9600);
  pinMode(ledStatePin, OUTPUT);
  pinMode(ledSignalPin, OUTPUT); 
  pinMode(controlledPin, OUTPUT); 
  Serial.println("system begins!");
  // set the data rate for the SoftwareSerial port
  XBee.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  randomSeed(analogRead(5));
  readXBeeDeviceId();
  toggle_time = millis();
  signal_time = millis();

}

void loop()
{
  if(irrecv.decode(&results)) {
    delay(5);
    DEBUG_PRINT("\nIR received: ");
    DEBUG_PRINTLN(results.value);
    sendBackDeviceID();
    irrecv.resume();
  }
  else if (XBee.available()) {
    delay(5);
    struct XBeePacket p = readXBeePacket(&XBee);
    printXBeePacket(p);
    if ( strcmp(p.type, "L") == 0 && strcmp(p.id, "FF") == 0) {
      // broadcast message
      sendBackDeviceID();
    }
    else if (atoi(p.id) == atoi(deviceId)) {
        // pass this message to the function of client
        lampClient(p);
    }
  }
}

void sendBackDeviceID() {
  randomDelay = random(1000);
  // avoid conflicts
  delay(randomDelay);
  // send back acknowledge packet
  sendXBeePacketFromRaw(&XBee, deviceId, "a", "0000");
  delay(2000);
}

void readXBeeDeviceId() {
  delay(500);

  memset(XBeeInString, 0, 50);
  DEBUG_PRINTLN("sending +++");
  XBee.print("+++");
  delay(3000);
  // DEBUG_PRINTLN("reading...");
  readStringfromSerial(&XBee, XBeeInString);
  // DEBUG_PRINTLN(XBeeInString);
  
  delay(1000);
  
  memset(XBeeInString, 0, 50);
  DEBUG_PRINTLN("sending ATMY");
  XBee.print("ATMY\r");
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);

  // deviceId = XBeeInString[];
  int id = atoi(XBeeInString);
  // string_copy(deviceId, XBeeInString, 0, 1);

  if(id<10) { //append 0 at beginning
    DEBUG_PRINTLN("id<10");
    deviceId[0] = '0';
    char a[2];
    itoa(id, a, 10);
    deviceId[1] = a[0];
  } else {
    itoa(id, deviceId, 10);  
  }

  Serial.print("my device ID: ");
  Serial.println(deviceId);  
}


void lampClient(struct XBeePacket p) {
  DEBUG_PRINTLN("command issued");
  if (strcmp(p.data, "  ON") == 0) {
    digitalWrite(controlledPin, HIGH);
  }
  else if (strcmp(p.data, " OFF") == 0) {
    digitalWrite(controlledPin, LOW);
  }
}

void laptopBridging(struct XBeePacket p) {
  DEBUG_PRINTLN("[CONNECTED] command issued");
  Serial.println(p.data);
}
