#include <SoftwareSerial.h>

#include "bt.h"
SoftwareSerial BT(10, 11); // RX, TX

int i = 0;

void setup(){
  Serial.begin(9600);
  // state = IDLE;
  // digitalWrite(softpotPin, HIGH); //enable pullup resistor
  // randomSeed(analogRead(5));
  // digitalWrite(ledPin, LOW);
  BT.begin(57600);
  // XBeePacketCounter = 0;
  // pressed = false;
  // released = false;
  // last_release_time = 0;
  
  delay(300);
  Serial.print("system begins");
}

char str[50];

int readBTLine(char *strArray) {
  int i=0, count;
  char c;
  while (BT.available() && (c = BT.read()) != '\n') {
    strArray[i] = c;
    i++;
  }
  strArray[i] = '\0';

  Serial.print("Read characters from BT, count:");
  Serial.println(i);
  count = i;
  return count;
}

void loop(){
  if(BT.available()){
    Serial.print("msg receive:");
    int msgLen = readBTLine(str);
    Serial.print(msgLen);
    Serial.print(" ");
    Serial.println(str);
  }
  if (i == 100000) {
    BT.write("send to BT\n");
    i = 0;
  }
  i++;

  // Serial.println("send to Serial");
  delay(500);

}