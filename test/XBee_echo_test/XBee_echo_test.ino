/*
  XBee_echo_test.ino
  
  Created 03/26/2014
  By benzh@eecs.berkeley.edu

*/

#include <SoftwareSerial.h>

SoftwareSerial XBee(3,2); // RX, TX


void setup()  
{
  Serial.begin(9600);
  XBee.begin(9600);

  Serial.println("ready!");
}

String serialString;
void loop()
{
   if (XBee.available()) {
     delay(2);                 //delay to allow buffer to fill 
      while (XBee.available()) {
        char c = XBee.read();  //gets one byte from serial buffer
        serialString += c; //makes the string readString
      }
     
     XBee.print(serialString);
     serialString = "";
   }
}

