#include <SoftwareSerial.h>
#include <IRremote.h>

SoftwareSerial XBee(2, 3); // RX, TX

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Serial begins");

  // set the data rate for the SoftwareSerial port
  XBee.begin(9600);
  
}

void loop() // run over and over
{
  if (XBee.available())
    Serial.write(XBee.read());
  if (Serial.available()) {
    Serial.write(Serial.read());
    XBee.write("!");
  }
    
}
