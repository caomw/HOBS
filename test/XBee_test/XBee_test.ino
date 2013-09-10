/*
  XBee test on Mega

  For old code that works with Aruidno Uno, see git log and pull it from the repo

  Modified upon "Mega multple serial test"
  
  Receives from the main serial port, sends to the others. 
  Receives from serial port 2 (XBee), sends to the main serial (Serial 0).
 
  This example works only on the Arduino Mega
 
  created 07/25/2013
  by Ben Zhang <benzh@eecs.berkeley.edu>
 
 */
#include <SoftwareSerial.h>

// #define XBee Serial2

SoftwareSerial XBee(3,2);
int ledStatePin = 13;
int ledSignalPin = 10;
int controlledPin = 12;
int ledTargetPin = 11;
char XBeeInString[50];

void setup() {
  // initialize both serial ports:
  pinMode(ledStatePin, OUTPUT);
  pinMode(ledSignalPin, OUTPUT); 
  pinMode(controlledPin, OUTPUT);
  pinMode(ledTargetPin, OUTPUT); 

  Serial.begin(9600);
  XBee.begin(9600);
  Serial.println("ready!");

  enterCommandMode();
  setID(14);
  setChannel(4321);
  readID();
  readChannel();

  writePermanent();

  Serial.println("done!");
  
}

void loop() {
  // read from port 1, send to port 0:
  // if (XBee.available()) {
  //   memset(XBeeInString, 0, 50);
  //   readStringfromSerial(&XBee, XBeeInString);
  //   Serial.println(XBeeInString);
  // }
  
  // read from port 0, send to port 1:
  // if (Serial.available()) {

  //   int inByte = Serial.read();
  //   Serial.print("input: ");
  //   Serial.println(inByte);
  //   XBee.print(inByte);
  //   // XBee.write('\n');
  // }

  // if(Serial.available()) {
  //   // get the new byte:
  //   char inChar = (char)Serial.read(); 
  //   // Serial.println(inChar);
  //   // add it to the inputString:
  //   inputString += inChar;
  //   // if the incoming character is a newline, set a flag
  //   // so the main loop can do something about it:
  //   if (inChar == '\n') {
  //     Serial.println("string complete");
  //     stringComplete = true;
  //   } 
  // }
  
  // if(stringComplete) {
  //   Serial.print("input: ");
  //   Serial.println(inputString); 
  //   // XBee.print(inputString);
  //     // clear the string:
  //   inputString = "";
  //   stringComplete = false;  
  // }
  
  digitalWrite(ledStatePin, HIGH);
  delay(500);
  digitalWrite(ledStatePin, LOW);
  delay(500);
  digitalWrite(ledSignalPin, HIGH);
  delay(500);
  digitalWrite(ledSignalPin, LOW);
  delay(500);
  digitalWrite(ledTargetPin, HIGH);
  delay(500);
  digitalWrite(ledTargetPin, LOW);
  delay(500);
}

// void serialEvent() {
//   while (Serial.available()) {
//     // get the new byte:
//     char inChar = (char)Serial.read(); 
//     // add it to the inputString:
//     inputString += inChar;
//     // if the incoming character is a newline, set a flag
//     // so the main loop can do something about it:
//     if (inChar == '\n') {
//       Serial.println("string complete");
//       stringComplete = true;
//     } 
//   }
// }

void enterCommandMode() {
  delay(2000);
  memset(XBeeInString, 0, 50);
  Serial.println("sending +++");
  XBee.print("+++");
  delay(3000);
  // DEBUG_PRINTLN("reading...");
  readStringfromSerial(&XBee, XBeeInString);
  // DEBUG_PRINTLN(XBeeInString);  
}

void readID() {
  delay(1000);  
  memset(XBeeInString, 0, 50);
  Serial.println("sending ATMY");
  XBee.print("ATMY\r");
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);

  // deviceId = XBeeInString[];
  // Serial.println(XBeeInString);
  int id = (int)strtol(XBeeInString, NULL, 16);
  // int id = int(XBeeInString);
  Serial.print("id in dec: ");
  Serial.println(id);
}

void setID(int id) {
  delay(1000);  
  memset(XBeeInString, 0, 50);
  Serial.print("setting ATMY: ");
  Serial.println(id);
  String hex = String(id, HEX);
  String cmd = "ATMY"+hex+"\r";
  XBee.print(cmd);
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);
  // Serial.println(XBeeInString);

}

void readChannel() {
  delay(1000);  
  memset(XBeeInString, 0, 50);
  Serial.println("sending ATID");
  XBee.print("ATID\r");
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);

  // deviceId = XBeeInString[];
  // Serial.println(XBeeInString);
  // int id = (int)strtol(XBeeInString, NULL, 16);
  // int id = int(XBeeInString);
  // Serial.println(id);
}

void setChannel(int ch) {
  delay(1000);  
  memset(XBeeInString, 0, 50);
  Serial.print("setting ATID: ");
  Serial.println(ch);
  String strCh = String(ch);
  String cmd = "ATID"+strCh+"\r";
  XBee.print(cmd);
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);
  // Serial.println(XBeeInString);

}

void writePermanent() {
  delay(1000);  
  memset(XBeeInString, 0, 50);
  Serial.println("setting ATWR");
  XBee.print("ATWR\r");
  delay(3000);
  readStringfromSerial(&XBee, XBeeInString);
}

int readStringfromSerial (SoftwareSerial *SS, char *strArray) {
  int i = 0;
  while ((*SS).available()) {
    strArray[i] = (*SS).read();
    i++;
  }
  strArray[i] = '\0';

  if (strArray[i-1] == '\n') {
    strArray[i-1] = '\0';
  }
  Serial.print("read message: ");
  Serial.print(strArray);
  Serial.print("  count: ");
  Serial.println(i);
  return i;
}