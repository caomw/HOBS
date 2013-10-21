/*
  IR test

  Used to test IR functionality.

  On Mega, you need to use pin 9
  
  Created 07/25/2013
  By benzh@eecs.berkeley.edu

*/

#include <IRremote.h>

// An IR LED must be connected to Arduino PWM pin 3.
// On Mega, you need to use pin 9
IRsend irsend;
int ledPin = 8;

void setup()
{
  Serial.begin(9600);
  delay(100);
  pinMode(9, OUTPUT);

  Serial.print("system begins!");
}

void loop() {
  delay(300);
  unsigned int session_id = 0xFFFF;
  Serial.print("[INIT] Sending IR: ");
  Serial.println(session_id);
  irsend.sendSony(session_id, 16);
}
