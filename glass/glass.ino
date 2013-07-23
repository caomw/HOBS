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
char message[50];
enum mode {
  BTMode,
  XBeeMode
};
mode m;

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  BT.begin(57600);
  delay(300);
  m = BTMode;
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
        m = XBeeMode;
        XBee.begin(9600);
        // TODO
        // better wait for a while before ending XBee session
        //
      }
      else {
        XBee.begin(9600);
        m = XBeeMode;
        XBee.print(message);
      }
    }
  }
  else if (m == XBeeMode) {
    if (XBee.available()) {
      DEBUG_PRINTLN("reading message from XBee");
      readStringfromSerial(&XBee, message);
      Serial.print(message);
      BT.begin(57600);
      BT.print(message);
      m = BTMode;
    }
  }
  delay(10);
}