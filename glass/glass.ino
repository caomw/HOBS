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

#undef DEBUG

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

// pin configuration
const int led_pin = 8;
const int ir_pin = 9;

// variables
int ir_rssi_max = 0;
int ir_current_rssi = 0;
char current_id[03] = "00";
bool is_increase_available = false;
bool is_connected = false;
char message[20];
unsigned long start_time;
boolean isWaitingReply;
char XBeeReturnIDs[20];
int XBeeReturnCount;
boolean ir_bcast_mode = true;
unsigned long ir_time;
unsigned long visual_cue_time;
unsigned int ir_cycle = 100;  // broadcat IR signal every X ms
unsigned int visual_cue_cycle = 300;  // how frequently update visual cue
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
  BT.begin(115200);

  // make sure the radio is ready
  delay(100);
  isWaitingReply = false;
  Serial.print("system begins!");
  visual_cue_time = millis();
  ir_time = millis();
}

void loop() {
  // in IR broadcast mode, we keep sending IR signal and hover the one with largeste intensity reading
  if (ir_bcast_mode) {
    // constantly sending out ir broadcast as visual cue
    if(millis() - ir_time > ir_cycle) {
      // -1 (0xFFFF) indicates for broacast which is different than normal session id
      irsend.sendSony(0xFFFF,16);
      ir_time = millis();
    }
    if(millis() - visual_cue_time > visual_cue_cycle) {
      // before sending out, we choose to see the last largest readings
      int current_selected = atoi(current_id);
      if (current_selected != 0) {
	XBee.write(current_id);
	XBee.write("H");
	XBee.write("XXX");
	XBee.println("XXX");
	ir_rssi_max = 0;
	visual_cue_time = millis();
      }
    }

    if (XBee.available()) {
      delay(10);
      while (XBee.available()) {
	readStringfromSerial(&XBee, message, false);
	DEBUG_PRINTLN(message);
	// for now the message could be defined as id + "x" + value
	// we parse the message and update the queue

	ir_current_rssi = atoi(message+3);

	if (ir_current_rssi > ir_rssi_max) {
	  // then we should update it
	  ir_rssi_max = ir_current_rssi;
	  current_id[0] = message[0];
	  current_id[1] = message[1];
	}
      }
    }
    if (BT.available()) {
      delay(10);
      Serial.print("[BT]: ");
      readStringfromSerial(&BT, message, true);
      Serial.println(message);
      // if it's LIST command, then list all available devices by sending IR
      if ( message[0] == 'F' && message[1] == 'F') {
	// return current_id to BT and set up connection to the client
	DEBUG_PRINT("sending back ID: ");
	DEBUG_PRINTLN(current_id);
	BT.println(current_id);
	XBee.write(current_id);
	XBee.write("C"); // click event
	XBee.write("XXX");
	XBee.println("XXX");
	
	is_connected = true;
	ir_bcast_mode = false;
      }
    }

  }
  else { // if(ir_bcast_mode){
    // this is the place when we only have a single connection with one device
    // TODO: implement later
    if (BT.available()) {
      delay(10);
      Serial.print("[BT]: ");
      readStringfromSerial(&BT, message, true);
      Serial.println(message);
      // if it's LIST command, then list all available devices by sending IR
      if ( message[0] == 'D') {
	// return current_id to BT and set up connection to the client
	DEBUG_PRINT("disconnect the clients");
	DEBUG_PRINTLN(current_id);
	// send out disconnection message to all the nodes
	XBee.write("00");
	XBee.write("D");
	XBee.write("XXX");
	XBee.println("XXX");

	is_connected = false;
	ir_bcast_mode = true;
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
