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

#define DEBUG

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

char deviceId[3] = "00";
char XBeeInString[50];

unsigned int ledStateInterval = 600;
unsigned long randomDelay = 0;
unsigned long start_time;
unsigned long end_time;
unsigned long toggle_time;
unsigned long signal_time;
unsigned long pendingThreshold = 10000;

#define statusOff 0
#define statusPending 1
#define statusOn 2

const char deviceLaptop[3] = "01";
const char deviceLamp[3] = "02";

boolean statePending = false;

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

  toggle_time = millis(); //for pending led purpose
}

void loop()
{


  if(statePending) {

    

    end_time = millis();

    if(end_time - toggle_time > ledStateInterval) {
      digitalToggle(ledStatePin);
      toggle_time = millis();
    }
  } 

  if(irrecv.decode(&results)) {
    delay(5);
    DEBUG_PRINT("\nIR received: ");
    DEBUG_PRINTLN(results.value);
    sendBackDeviceID();
    irrecv.resume();

    //client starts blinking
    statePending = true;
    ledStateInterval = 500;

  }
  else if (XBee.available()) {
    delay(5);
    struct XBeePacket p = readXBeePacket(&XBee);
    printXBeePacket(p);
    if ( strcmp(p.id, "FF") == 0 ) {
      // broadcast message
      sendBackDeviceID();
    }
    else if(strcmp(p.var, "SEL") == 0) {
      // if it's selection related, process in this level
      DEBUG_PRINTLN("selection msg received");
      if(strcmp(p.data, " ON") == 0) {
        //one is selected => turn on led
        //the rest => turn off led
        if(atoi(p.id) == atoi(deviceId)) {
          digitalWrite(ledStatePin, HIGH);  
        } else {
          digitalWrite(ledStatePin, LOW);  
        }
        statePending = false;

      } else if(strcmp(p.data, "OFF") == 0) {
        //turn off all the led just in case

        digitalWrite(ledStatePin, LOW);
        statePending = false;

      } else if(strcmp(p.data, "080") == 0) {
        //the one is hovered => turn on
        //the rest => blink slow
        if(atoi(p.id) == atoi(deviceId)) {
          statePending = false;
        } else {
          ledStateInterval = 500;
          statePending = true;
        }
        
        

      } 
    }
    else if (atoi(p.id) == atoi(deviceId)) {
      
      // pass this message to the function of client
      if(strcmp(deviceId, deviceLaptop) == 0) {
        laptopBridging(p);
      } else if(strcmp(deviceId, deviceLamp) == 0) {
        lampClient(p);
      }
    }
  }
}

void sendBackDeviceID() {
  randomDelay = random(1000);
  // avoid conflicts
  delay(randomDelay);
  // send back acknowledge packet
  sendXBeePacketFromRaw(&XBee, deviceId, "A", " ID", "XXX");
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
  if (strcmp(p.func, "R") == 0 && strcmp(p.var, "BRI") == 0 ) {
    // read the current status and reply
    int status = digitalRead(controlledPin);
    if (status == 0) {
      sendXBeePacketFromRaw(&XBee, deviceId, "A", "BRI", "OFF");
    } else {
      sendXBeePacketFromRaw(&XBee, deviceId, "A", "BRI", " ON");
    }
  }
  else if (strcmp(p.func, "C") == 0 && strcmp(p.var, "BRI") == 0 && strcmp(p.data, " ON") == 0) {
    DEBUG_PRINTLN("turned on");
    digitalWrite(controlledPin, HIGH);
    sendXBeePacketFromRaw(&XBee, deviceId, "A", "BRI", " ON");
  }
  else if (strcmp(p.func, "C") == 0 && strcmp(p.var, "BRI") == 0 && strcmp(p.data, "OFF") == 0) {
    DEBUG_PRINTLN("turned off");
    digitalWrite(controlledPin, LOW);
    sendXBeePacketFromRaw(&XBee, deviceId, "A", "BRI", "OFF");
  }
  else {
    // error message
    sendXBeePacketFromRaw(&XBee, deviceId, "E", "XXX", "XXX");
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


  char strArray[20];
  int i = 0;

  if(p.func[0] == 'R') {
    delay(800);
    // read the serial return value, and return back message
    while (Serial.available()) {
      strArray[i] = Serial.read();
      i++;
    }
    strArray[i] = '\0';
    XBee.println(strArray);
  } else {
    // sendXBeePacketFromRaw(&XBee, deviceId, "A", p.var, p.data);
  }   
}
