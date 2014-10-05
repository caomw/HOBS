/*
  client.ino

  This is a general framework to fit in any clients that can be remotely
  controlled with XBee. Each clients will add additional function of parsing
  certain commands, while the major application takes charge of external
  communication.

  Examples of clients include, for now, lamps and laptops.

  Lamps are simple, dump clients that only support on/off. Laptops now are
  viewed as video players, thus supporting play/pause, volume adjustment, and
  brightness adjustment, etc.


  Created 07/17/2013, modified on 10/05/2014
  By benzh@eecs.berkeley.edu

*/

#include <SoftwareSerial.h>
#include <IRremote.h>
#include <string.h>

#include "utils.h"

// uncomment for debugging
// #define DEBUG
// #define DEBUG_TAG

SoftwareSerial XBee(3,2); // RX, TX

const int led_state_pin = 11;
const int led_signal_pin = 10;
const int control_pin = 12;
const int led_target_pin = 13;
const int tsl267_pin = 5;
const int ir_rcv_pin = 8;

IRrecv irrecv(ir_rcv_pin);
decode_results results;

char deviceId[3] = "00";
char XBeeInString[50];

// other variables
int ir_rssi = 0;
int ir_rssi_current_max = 0;
int ir_rssi_last_max = 0;
int ir_rssi_increase = 0;
boolean blinkShort = false;
unsigned int blinkShort_ratio = 6;
unsigned int ledStateInterval = 200;
unsigned long randomDelay = 0;
unsigned long start_time;
unsigned long end_time;
unsigned long toggle_time;
unsigned long signal_time;
unsigned long signal_threshold = 310;
unsigned long pendingThreshold = 10000;
int random_mutiplier = 15;
int bucket = 0;
#define statusOff 0
#define statusPending 1
#define statusOn 2


char message[20];

// Hard coded IDs for appliances
const char deviceTV[3] = "12";
const char deviceMusic[3] = "13";
const char deviceLamp[3] = "11";
const char deviceFan[3] = "14";


boolean statePending = false;
boolean signal_response = false;

// Parameter used for EXPERIMENT
const int MODE_IR = 1;
const int MODE_LIST = 2;
int exp_mode = MODE_IR;
boolean notMe = false;

void setup()
{
  Serial.begin(9600);
  Serial.println("Configuring...");
  pinMode(led_state_pin, OUTPUT);
  pinMode(led_signal_pin, OUTPUT);
  pinMode(control_pin, OUTPUT);
  pinMode(led_target_pin, OUTPUT);
  pinMode(tsl267_pin, INPUT);

  // set the data rate for the SoftwareSerial port
  XBee.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  randomSeed(analogRead(5));
  readXBeeDeviceId();

  toggle_time = millis(); //for pending led purpose
  signal_time = millis();
  Serial.print("random_mutiplier: ");
  Serial.println(random_mutiplier);
  Serial.println("ready!");

  for (int i = 0; i < 3; ++i) {
    digitalWrite(led_state_pin, HIGH);
    digitalWrite(led_signal_pin, HIGH);
    digitalWrite(led_target_pin, HIGH);
    delay(100);
    digitalWrite(led_state_pin, LOW);
    digitalWrite(led_signal_pin, LOW);
    digitalWrite(led_target_pin, LOW);
    delay(100);
  }
}

void loop()
{
  // results.value = 0xFFFE;

  // irrecv.decode will return non zero result when the data is ready.
  // before that we should keep polling the data from the tsl267_pin
  // and tracks the maximum
  ir_rssi = analogRead(tsl267_pin);
  if (ir_rssi > ir_rssi_current_max)
    ir_rssi_current_max = ir_rssi;

  if(irrecv.decode(&results)) {
    delay(5);
    Serial.print("IR: ");
    Serial.print(results.value);
    Serial.print(", ");
    Serial.println(ir_rssi_current_max);
    ir_rssi_increase = ir_rssi_current_max - ir_rssi_last_max;
    ir_rssi_last_max = ir_rssi_current_max;
    ir_rssi_current_max = 0;

    if(results.value == 0xFFFF){
      // send the rssi back through XBee
      // digitalWrite(led_signal_pin, HIGH);
      // think about how to do the scheduling such that there is no conflict

      if (ir_rssi_increase > 50) {
	XBee.println(deviceId + String("i") + String(ir_rssi_last_max));
      }
      else if (ir_rssi_increase < -50) {
	delay(100);
	XBee.println(deviceId + String("d") + String(ir_rssi_last_max));
      }
      else {
	delay((atoi(deviceId) % 3 + 1) * 30);
	XBee.println(deviceId + String("u") + String(ir_rssi_last_max));
      }

      // turn on LED
      signal_time = millis();
      digitalWrite(led_signal_pin, HIGH);
      signal_response = true;


    } else if(results.value <= 0x32 && results.value > 0){
      // limit the session ID to be a random number between 0~50
      sendBackDeviceID();
      //setting itself to pending state and start blinking slow
      statePending = true;
      blinkShort = true;
    } else {
      //garbage message
    }
    irrecv.resume();
  }

  if(signal_response){
    if(millis() - signal_time > signal_threshold){
      digitalWrite(led_signal_pin, LOW);
      signal_response = false;
    }
  }

  if (XBee.available()) {
    delay(5);
    readStringfromSerial(&XBee, message, true);

    // the led only lights up when it receives commands from the master
    if ( message[0] == 'H') {   // hover
      digitalWrite(led_state_pin, LOW);
      // turn on LED
      if (message[1] == deviceId[0] && message[2] == deviceId[1]) {
  	digitalWrite(led_signal_pin, HIGH);
	signal_response = false;
      }
      else {
  	digitalWrite(led_signal_pin, LOW);
	delay(100);
      }
    }

    // the led only lights up when it receives commands from the master
    else if ( message[0] == 'C') {   // control
      // turn on LED
      if (message[1] == deviceId[0] && message[2] == deviceId[1]) {
	digitalWrite(led_state_pin, HIGH);
      }
      else {
	digitalWrite(led_state_pin, LOW);
      }
      digitalWrite(led_signal_pin, LOW);
      digitalWrite(led_target_pin, LOW);
    }

    // the led only lights up when it receives commands from the master
    else if ( message[0] == 'L') {   // light up for targeting
      // turn on LED
      if (message[1] == deviceId[0] && message[2] == deviceId[1]) {
	digitalWrite(led_target_pin, HIGH);
      }
      else {
	digitalWrite(led_target_pin, LOW);
      }
      digitalWrite(led_signal_pin, LOW);
      digitalWrite(led_state_pin, LOW);
    }

    else if ( message[0] == 'D') {   // disconnect, reset
      // turn off LED
      digitalWrite(led_state_pin, LOW);
      digitalWrite(led_target_pin, LOW);
      digitalWrite(led_signal_pin, LOW);
    }
  }
}

void sendBackDeviceID() {
  // randomDelay = random(500);
  // avoid conflicts

  if(atoi(deviceId) >10) {
    randomDelay = (atoi(deviceId) - 10) * random_mutiplier;
    // if it's 11 - 14 ==> test 2
  } else {
    randomDelay = atoi(deviceId) * random_mutiplier;
  }
  delay(randomDelay);
  DEBUG_TAGGING(randomDelay, " delay, sending back device ID\n" );
  // send back acknowledge packet
  sendXBeePacketFromRaw(&XBee, deviceId, "A", " ID", "XXX");

}

void readXBeeDeviceId() {
  delay(100);
  memset(XBeeInString, 0, 50);
  DEBUG_PRINTLN("sending +++");
  XBee.print("+++");
  delay(2500);
  // DEBUG_PRINTLN("reading...");
  readStringfromSerial(&XBee, XBeeInString);
  // DEBUG_PRINTLN(XBeeInString);
  delay(500);
  memset(XBeeInString, 0, 50);
  DEBUG_PRINTLN("sending ATMY");
  XBee.print("ATMY\r");
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);

  // deviceId = XBeeInString[];
  Serial.println(XBeeInString);
  int id = (int)strtol(XBeeInString, NULL, 16);
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

void powerClient(struct XBeePacket p) {
  DEBUG_PRINTLN("command issued");
  // if (strcmp(p.func, "R") == 0 && strcmp(p.var, "POW") == 0 ) {
  //   // read the current status and reply
  //   int status = digitalRead(control_pin);
  //   if (status == 0) {
  //     sendXBeePacketFromRaw(&XBee, deviceId, "A", "POW", "OFF");
  //   } else {
  //     sendXBeePacketFromRaw(&XBee, deviceId, "A", "POW", " ON");
  //   }
  // }
  //taking out because now it auto reply when selected, glass won't send R msg
  if (strcmp(p.func, "C") == 0 &&
	  strcmp(p.var, "POW") == 0 &&
	  strcmp(p.data, " ON") == 0) {
    DEBUG_PRINTLN("turned on");
    digitalWrite(control_pin, HIGH);
    sendXBeePacketFromRaw(&XBee, deviceId, "A", "POW", " ON");
  }
  else if (strcmp(p.func, "C") == 0 &&
		   strcmp(p.var, "POW") == 0 &&
		   strcmp(p.data, "OFF") == 0) {
    DEBUG_PRINTLN("turned off");
    digitalWrite(control_pin, LOW);
    sendXBeePacketFromRaw(&XBee, deviceId, "A", "POW", "OFF");
  }
  else {
    // error message
    sendXBeePacketFromRaw(&XBee, deviceId, "E", "XXX", "XXX");
  }
}

void replyStatus() {
  DEBUG_PRINTLN("ask for client status");
  if(strcmp(deviceId, deviceLamp) == 0 || strcmp(deviceId, deviceFan) == 0) {
    int status = digitalRead(control_pin);
    if (status == 0) {
      sendXBeePacketFromRaw(&XBee, deviceId, "A", "POW", "OFF");
    } else {
      sendXBeePacketFromRaw(&XBee, deviceId, "A", "POW", " ON");
    }
  } else if (strcmp(deviceId, deviceTV) == 0 ||
			 strcmp(deviceId, deviceMusic) == 0) {
    //send a read status cmd to the laptop and wait for reply
    //curently only volume is needed
    Serial.print(deviceId);
    Serial.println("RVOLXXX");
    delay(500);
    char strArray[20];
    int i = 0;
    // read the serial return value, and return back message
    while (Serial.available()) {
      strArray[i] = Serial.read();
      i++;
    }
    strArray[i] = '\0';
    XBee.println(strArray);
  }
}

void laptopBridging(struct XBeePacket p) {
  // make sure you send back ack
  DEBUG_PRINTLN("command issued");
  char str[20];
  string_concat(str, p.id, 0);
  string_concat(str, p.func, 2);
  string_concat(str, p.var, 3);
  string_concat(str, p.data, 6);
  str[9] = '\0';
  Serial.println(str);

  // char strArray[20];
  // int i = 0;

  // if(p.func[0] == 'R') {
  //   delay(500);
  //   // read the serial return value, and return back message
  //   while (Serial.available()) {
  //     strArray[i] = Serial.read();
  //     i++;
  //   }
  //   strArray[i] = '\0';
  //   XBee.println(strArray);
  // } else {
  //   // sendXBeePacketFromRaw(&XBee, deviceId, "A", p.var, p.data);
  // }
}

int readStringfromSerial (SoftwareSerial *SS, char *strArray, bool debug) {
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
