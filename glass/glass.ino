/*
  glass.ino

  This code is running on the glass, taking control of IR tranmission, XBee communication, and BT communication.

  XBee messages from client are routed to Google Glass for display purpose.
  BT messages from Google Glass either trigger IR transmission, or are being delivered to clients. As before, we are using XBee broadcast mechanism.

  Now we've switched to Arduino Mega, no need for software serial. For old code, using git log to find it... 

  Created 07/17/2013
  Modified 07/25/2013
  By benzh@eecs.berkeley.edu

*/

#include <IRremote.h>

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif


#define BT Serial1
#define XBee Serial2

// An IR LED must be connected to Arduino PWM pin 3.
IRsend irsend;
int ledPin = 8;

char message[50];

unsigned long start_time;
boolean isWaitingReply;
char XBeeReturnIDs[20];
int XBeeReturnCount;

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBee.begin(9600);
  BT.begin(57600);
  isWaitingReply = false;
  delay(100);
  Serial.print("system begins!");
}

void loop() {
  if (isWaitingReply) {
    if (XBee.available()) {
      if (XBeeReturnCount >= 1) {
        XBeeReturnIDs[XBeeReturnCount*3-2] = ':';
      }
      delay(10);
      readStringfromSerial(&XBee, message);
      if (true == isPacketValid(message)) {
        XBeeReturnIDs[XBeeReturnCount*3] = message[0];
        XBeeReturnIDs[XBeeReturnCount*3+1] = message[1];
        XBeeReturnCount++;
      }
    }
       
    if (millis() - start_time > 1000) {
      if (XBeeReturnCount > 0)
        XBeeReturnIDs[XBeeReturnCount*3-1] = '\0';
      else
        XBeeReturnIDs[0] = '\0';
      DEBUG_PRINT("IDs list:");
      DEBUG_PRINTLN(XBeeReturnIDs);
      DEBUG_PRINT("IDs counts:");
      DEBUG_PRINTLN(XBeeReturnCount);
      BT.println(XBeeReturnIDs);
      isWaitingReply = false;      
    }
  }
      
  if (BT.available()) {
    // when receive message from Bluetooth, only trigger IR if
    // message is FFL_____, otherwise, use XBee to relay the message
    delay(10);
    DEBUG_PRINT("[BT]: ");
    readStringfromSerial(&BT, message);
    // if it's LIST command, then list all available devices by sending IR
    if ( message[0] == 'F' && message[1] == 'F') {
      unsigned int session_id = random(0xFFFF);
      DEBUG_PRINT("[INIT] Sending IR: ");
      DEBUG_PRINTLN(session_id);
      irsend.sendSony(session_id, 16);
      start_time = millis();
      memset(XBeeReturnIDs, 0, 20);
      XBeeReturnCount = 0;
      isWaitingReply = true;
    }
    else if (true == isPacketValid(message)) {
      XBee.print(message);
    }
  }

  if (XBee.available()) {
    delay(10);
    DEBUG_PRINT("[XBee]: ");
    readStringfromSerial(&XBee, message);
    DEBUG_PRINT("[BT]: send ");
    DEBUG_PRINTLN(message);    
    BT.println(message);
  }
  delay(10);
}

boolean isPacketValid(char *message) {
  // check message format
  // IDXVARVAL
  if (strlen(message) >= 9 && isFuncValid(message) && 
      (message[1] <= '9' && message[1] >= '0') &&
      (message[1] <= '9' && message[1] >= '0'))
    return true;
  return false;
}

boolean isFuncValid(char *message) {
  if (message[2] == 'R' || message[2] == 'A' ||
      message[2] == 'C' || message[2] == 'S')
    return true;
  return false;
}

int readStringfromSerial (HardwareSerial *SS, char *strArray) {
  int i = 0;
  while ((*SS).available()) {
    strArray[i] = (*SS).read();
    i++;
  }
  strArray[i] = '\0';
  if (strArray[i-1] == '\n') {
    strArray[i-1] = '\0';
  }
  DEBUG_PRINT("read message: ");
  DEBUG_PRINT(strArray);
  DEBUG_PRINT("  count: ");
  DEBUG_PRINTLN(i);
  return i;
}
