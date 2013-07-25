/*
  glass.ino

  This code is running on the glass, taking control of IR tranmission, XBee communication, and BT communication.

  XBee messages from client are routed to Google Glass for display purpose.
  BT messages from Google Glass either trigger IR transmission, or are being delivered to clients. As before, we are using XBee broadcast mechanism.

  Created 07/17/2013
  By benzh@eecs.berkeley.edu

*/

#include <SoftwareSerial.h>
#include <IRremote.h>

#define DEBUG
#include "utils.h"

SoftwareSerial BT(10, 11); // RX, TX
SoftwareSerial XBee(2, 4); // RX, TX


// An IR LED must be connected to Arduino PWM pin 3.
IRsend irsend;
int ledPin = 8;

struct XBeePacket XBeePacketArr[20];
char XBeeReturnIDs[20];
int XBeeReturnCount;

char message[50];
enum mode {
  BTMode,
  XBeeMode,
  WaitMode
};
mode m;
unsigned long start_time;

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  BT.begin(57600);
  delay(300);
  m = BTMode;
  XBeeReturnCount = 0;
  Serial.print("system begins");
}

void loop() {
  if (m == BTMode) {
    if (BT.available()) {
      
      // when receive message from Bluetooth, only trigger IR if
      // message is FFL_____, otherwise, use XBee to relay the message
      readStringfromSerial(&BT, message);
      char cmd[2+1];
      string_copy(cmd, message, 0, 1);
      // if it's LIST command, then list all available devices by sending IR
      if (strcmp(cmd, "FF") == 0) {
        unsigned int session_id = random(0xFFFF);
        DEBUG_PRINT("[INIT] Sending IR: ");
        DEBUG_PRINTLN(session_id);
        irsend.sendSony(session_id, 16);
        XBeeReturnCount = 0;
        memset(XBeeReturnIDs, '\0', 20);
        start_time = millis();
        DEBUG_PRINTLN("entering wait mode");
        m = WaitMode;
        XBee.begin(9600);
      }
      else if (true == isPacketValid(message)) {
        XBee.begin(9600);
        DEBUG_PRINTLN("entering XBee mode");
        start_time = millis();
        m = XBeeMode;
        XBee.print(message);
      }
    }
  }
  else if (m == XBeeMode) {
    delay(30);
    // too long time no response
    if (millis() - start_time > 3000) {
      BT.begin(57600);
      DEBUG_PRINTLN("timeout, entering BT mode");
      m = BTMode;
    }
    if (XBee.available()) {
      DEBUG_PRINTLN("reading message from XBee");
      readStringfromSerial(&XBee, message);
      Serial.print(message);
      BT.begin(57600);
      BT.print(message);
      DEBUG_PRINTLN("entering BT mode");
      m = BTMode;
    }
    
  }
  else if (m == WaitMode) {
    if (XBee.available()) {
      if (XBeeReturnCount >= 1) {
        XBeeReturnIDs[XBeeReturnCount*3-2] = ':';
        DEBUG_PRINT("appending :");
        DEBUG_PRINTLN(XBeeReturnCount);
      }
      readStringfromSerial(&XBee, message);
      XBeeReturnIDs[XBeeReturnCount*3] = message[0];
      XBeeReturnIDs[XBeeReturnCount*3+1] = message[1];
      XBeeReturnCount++;
    }
    if (millis() - start_time > 1000) {
      if (XBeeReturnCount > 0)
        XBeeReturnIDs[XBeeReturnCount*3-1] = '\0';
      else
        XBeeReturnIDs[0] = '\0';

b      DEBUG_PRINT("IDs list:");
      DEBUG_PRINTLN(XBeeReturnIDs);
      DEBUG_PRINT("IDs counts:");
      DEBUG_PRINTLN(XBeeReturnCount);
      BT.print(XBeeReturnIDs);
      m = BTMode;
      DEBUG_PRINTLN("entering BT mode");

      BT.begin(57600);
    }
  }  
  delay(10);
}

boolean isPacketValid(char *message) {
  // check message format
  // IDXVARVAL
  if (strlen(message) >= 9 &&
      (message[2] == 'R' || message[2] == 'S' || message[2] == 'C') && 
      (message[1] <= '9' && message[1] >= '0') &&
      (message[1] <= '9' && message[1] >= '0'))
    return true;
  return false;
}