#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11); // RX, TX


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
  Serial.println("system begins");

  BT.println("ATVER,ver1");
  delay(100);
  while(BT.available()){
    // Serial.print("msg receive:");
    Serial.write(BT.read());
    // Serial.println();
  }

  delay(100);

  BT.println("ATRSW,6");
  delay(100);
  while(BT.available()){
    // Serial.print("msg receive:");
    Serial.write(BT.read());
    // Serial.println();
  }

  delay(100);

  // BT.println("ATPAIR,B8F6B1196970");
  // delay(2000);
  // while(BT.available()){
  //   // Serial.print("msg receive:");
  //   Serial.write(BT.read());
  //   // Serial.println();
  // }
}

void loop(){
	if(BT.available()){
		// Serial.print("msg receive:");
		Serial.write(BT.read());
    // Serial.println();
	}
  // BT.write("send to BT\n");
  // Serial.println("send to Serial");
  if(Serial.available()){
    // char msg[100];
    
    // Serial.readBytesUntil('\n', msg, 100);
    // Serial.print("send via BT");
    // String msg_str = new String(msg);
    // Serial.println(msg_str);
    // BT.print(msg_str);
    // BT.write(msg);

    BT.write(Serial.read());
    BT.println();
  }
  // delay(10); 

}