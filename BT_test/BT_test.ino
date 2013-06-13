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
  Serial.print("system begins");
}

void loop(){
	if(BT.available()){
		Serial.print("msg receive:");
		Serial.write(BT.read());
    Serial.println();
	}
  BT.write("send to BT\n");
  // Serial.println("send to Serial");
  delay(500);

}