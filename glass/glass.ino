/*
  glass.ino

  This code is running on the glass, taking control of IR tranmission, XBee communication, and BT communication.

  XBee messages from client are routed to Google Glass for display purpose.
  BT messages from Google Glass either trigger IR transmission, or are being delivered to clients. As before, we are using XBee broadcast mechanism.

  Now we've switched to Arduino Mega, no need for software serial. For old code, using git log to find it... 

  Created  : 07/17/2013
  Modified : 03/25/2014
  Author   : Ben Zhang <benzh@eecs.berkeley.edu>

*/

#include <IRremote.h>

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_TAGGING(x, y)  Serial.print(x); Serial.println(y)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// For Arduino Mega, we are using the hardware serial, which provides best performance
#define BT Serial1
#define XBee Serial2

// For Arduino Mega, we connect an IR LED to pin 9
IRsend irsend;

const int led_pin = 8;
const int ir_pin = 9;

char message[20];
unsigned long start_time;
boolean isWaitingReply;
char XBeeReturnIDs[20];
int XBeeReturnCount;
boolean ir_bcast_mode = true;
unsigned long ir_time;
unsigned int ir_cycle = 100;  // broadcat IR signal every X ms
unsigned int ir_response_threshold = 800;

void setup()
{
  // Mega uses pin 9 for IR
  pinMode(ir_pin, OUTPUT);  
  pinMode(led_pin, OUTPUT);

  digitalWrite(led_pin, LOW);

  // set up serials
  Serial.begin(9600);
  XBee.begin(9600);
  BT.begin(57600);

  // make sure the radio is ready
  delay(100);
  isWaitingReply = false;
  Serial.print("system begins!");
  ir_time = millis();
}

void loop() {
  if(ir_bcast_mode){
    // constantly sending out ir broadcast as visual cue
    if(millis() - ir_time > ir_cycle) {
      // -1 (0xFFFF) indicates for broacast which is different than normal session id
      irsend.sendSony(0xFFFF,16);
      ir_time = millis();
    }
    delay(10);
    if (XBee.available()) {
      delay(10);
      while (XBee.available()) {
	readStringfromSerial(&XBee, message, false);
	DEBUG_PRINTLN(message);
	// for now the message could be defined as id + "n" + value
      }
    }
  }
}

boolean isPacketValid(char *message) {
  // check message format
  // IDXVARVAL
  if (strlen(message) >= 9 && isFuncValid(message) && 
      (message[0] <= '9' && message[0] >= '0') &&
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

int  readStringfromSerial (HardwareSerial *SS, char *strArray, bool debug) {
  int i = 0;
  while ((*SS).available() && i < 20) {
    strArray[i] = (*SS).read();
    if (strArray[i] == '\n') {
      break;
    }
    i++;
  }
  strArray[i] = '\0';
  if (strArray[i-1] == '\n') {
    strArray[i-1] = '\0';
  }
  if (debug) {
    DEBUG_PRINT("read message: ");
    DEBUG_PRINT(strArray);
    DEBUG_PRINT("  count: ");
    DEBUG_PRINTLN(i);
  }
  return i;
}
