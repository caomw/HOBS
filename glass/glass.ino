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
  #define           DEBUG_PRINTLN(x)  Serial.println(x)
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

#define MAXIMUM_TARGETS 30

// maybe we should do this, with smoothing
int intensity_array[MAXIMUM_TARGETS] = {0};

// variables
int ir_rssi_max = 0;
int ir_current_rssi = 0;
char current_id[03] = "00";
bool is_increase_available = false;
bool is_connected = false;
char message[20];
unsigned long start_time;
boolean isWaitingReply;
char IRReplies[50];
boolean ir_bcast_mode = true;
unsigned long ir_time;
unsigned long visual_cue_time;
unsigned long keep_history_time;
unsigned int ir_cycle = 100;  // broadcat IR signal every X ms
unsigned int visual_cue_cycle = 300;  // how frequently update visual cue
unsigned int keep_history_cycle = 800;  // how frequently update visual cue
unsigned int ir_response_threshold = 800;
unsigned int ir_reply_count = 0;

boolean is_control_from_glass = false;

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
  keep_history_time = millis();
  ir_time = millis();
  memset(IRReplies, 0, 50);

  for (int i = 0; i < 3; ++i) {
    digitalWrite(led_pin, HIGH);
    delay(100);
    digitalWrite(led_pin, LOW);
    delay(100);
  }
  XBee.println("D");

}

void loop() {
  // in IR broadcast mode, we keep sending IR signal and hover the one with largeste intensity reading
  if (ir_bcast_mode) {
    // constantly sending out ir broadcast as visual cue
    if(millis() - ir_time > ir_cycle) {
      // -1 (0xFFFF) indicates for broacast which is different than normal session id
      irsend.sendSony(0xFFFF,16);
      for (int i = 0; i < MAXIMUM_TARGETS; ++i)
	intensity_array[i] = 0.8 * intensity_array[i];
      ir_time = millis();
    }
    /* if (millis() - keep_history_time > keep_history_cycle) { */
    /*   print_intensity_arrays(intensity_array, MAXIMUM_TARGETS); */
    /* } */
    if(millis() - visual_cue_time > visual_cue_cycle) {
      // before sending out, we choose to see the largest
      int largest_intensity = 0;
      int largest_id = 0;
      for (int i = 1; i < MAXIMUM_TARGETS; ++i)
	if (intensity_array[i] > largest_intensity) {
	  largest_intensity = intensity_array[i];
	  largest_id = i;
	}
      
      if (largest_intensity > 10) {	
	DEBUG_PRINT("sending XBee to ");
	DEBUG_PRINT(largest_id);
	DEBUG_PRINT(" with intensity: ");
	DEBUG_PRINTLN(largest_intensity);

	XBee.write("H");
	if (largest_id < 10) {
	  XBee.write('0');
	  char id = '0' + largest_id;
	  XBee.println(id);
	}
	else {
	  char id = '0' + largest_id/10;
	  XBee.write(id);
	  id = '0' + largest_id%10;
	  XBee.write(id);
	}
      }
      else {
	XBee.println("H00"); // hover nothing
      }

      visual_cue_time = millis();
    }

    if (XBee.available()) {
      delay(10);
      while (XBee.available()) {
	readStringfromSerial(&XBee, message, false);
	DEBUG_PRINTLN(message);

	// for now the message could be defined as id + "x" + value
	// we parse the message and update the queue
	message[2] = '\0';
	int id = atoi(message);
	intensity_array[id] = 0.1 * intensity_array[id]  + 0.9 * atoi(message+3);
	
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
      Serial.print("[fromBT]: ");
      readStringfromSerial(&BT, message, true);
      Serial.println(message);
      // if it's LIST command, then list all available devices by sending IR
      if ( message[0] == 'F' && message[1] == 'F' && 
	   !(current_id[0] == '0' && current_id[1] == '0') ) {
	// return current_id to BT and set up connection to the client

	Serial.print("[toBT]: ");
	// print_intensity_arrays(&Serial, intensity_array, MAXIMUM_TARGETS);
	print_intensity_arrays(&BT, intensity_array, MAXIMUM_TARGETS);

      } else if ( message[0]== 'H') {
	XBee.println(message); // hover event
	delay(10);
	XBee.println(message); // hover event
	Serial.println(message); // hover
	ir_bcast_mode = false;
      }
      else if ( message[0]== 'C') {
	XBee.println(message); // click event
	delay(10);
	XBee.println(message); // click event
	is_connected = true;
	ir_bcast_mode = false;
      }

    }
  }
  else {    // if(ir_bcast_mode) 
    if (BT.available()) {
      delay(10);
      Serial.print("[BT]: ");
      readStringfromSerial(&BT, message, true);
      Serial.println(message);

      if ( message[0]== 'H') {
	XBee.println(message); // hover event
	delay(10);
	XBee.println(message); // hover event
	delay(10);
	XBee.println(message); // hover event
	Serial.write(message);
      } else if ( message[0]== 'C') {
	XBee.println(message); // click event
	delay(10);
	XBee.println(message); // click event
	is_connected = true;
      } else if ( message[0] == 'D') {
	// return current_id to BT and set up connection to the client
	DEBUG_PRINT("disconnect the clients");
	DEBUG_PRINTLN(current_id);
	// send out disconnection message to all the nodes
	XBee.write("D");
	is_connected = false;
	ir_bcast_mode = true;
      }
    }
  }
  if (Serial.available()) {
    if (Serial.read() == 'R') {
      ir_bcast_mode = true;
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

void print_intensity_arrays(HardwareSerial *HS, int array[], int n) {
  boolean new_line = false;
  int max = 0;
  for (int i = 1; i < n; ++i) {
    if (array[i] > max) {
      max = array[i];
    }
  }
  for (int i = 1; i < n; ++i) {
    if (array[i] > 10 && array[i] > max / 2) {
      (*HS).print(i);
      (*HS).print(":");
      (*HS).print(array[i]);
      (*HS).print(", ");
      new_line = true;
    }
  }
  if (new_line) 
    (*HS).println();
}
