/*
  XBee test 

  This code now takes charge of interfacing with XBee radio through Arduino Uno's software serial

  The idea is to execute AT command.

  created  : 07/25/2013
  modified : 03/26/2013
  author   : Sean Chen <sean.yhc@gmail.com>
 
 */
#include <SoftwareSerial.h>

SoftwareSerial XBee(3,2);
int ledStatePin = 13;
int ledSignalPin = 10;
int controlledPin = 12;
int ledTargetPin = 11;
char XBeeInString[50];

void setup() {
  pinMode(ledStatePin, OUTPUT);
  pinMode(ledSignalPin, OUTPUT); 
  pinMode(controlledPin, OUTPUT);
  pinMode(ledTargetPin, OUTPUT); 

  // initialize both serial ports:
  Serial.begin(9600);
  XBee.begin(9600);
  Serial.println("ready!");

  // uncomment the one that you want to use
  enterCommandMode();
  // setID(13);
  // setChannel(4321);
  readID();
  readChannel();
  writePermanent();
  exitCommandMode();

  Serial.println("done!");
}

void loop() {
  // some useless loop, just indicating the code is running
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

void exitCommandMode() {
  delay(1000);
  memset(XBeeInString, 0, 50);
  Serial.println("sending ATCN");
  XBee.print("ATCN\r");
  delay(2000);
  // DEBUG_PRINTLN("reading...");
  readStringfromSerial(&XBee, XBeeInString);
  // DEBUG_PRINTLN(XBeeInString);  
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
