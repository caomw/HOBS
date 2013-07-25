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

#define XBee Serial2

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  XBee.begin(9600);
}

void loop() {
  // read from port 1, send to port 0:
  if (XBee.available()) {
    int inByte = XBee.read();
    Serial.write(inByte); 
  }
  
  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    XBee.write(inByte);
    XBee.write('\n');
  }
}