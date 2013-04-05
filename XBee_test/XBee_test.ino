#include <SoftwareSerial.h>
#include <IRremote.h>

SoftwareSerial XBee(2, 3); // RX, TX
int ledPin = 13;
int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);   
  Serial.println("system begins!");

  // set the data rate for the SoftwareSerial port
  XBee.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() // run over and over
{
  if (XBee.available()) {
    Serial.write(XBee.read());
    
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    XBee.write("roger");
  }
  
  if (Serial.available()) {
    Serial.write(Serial.read());
    XBee.println("msg from serial");
  }
  
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    XBee.print("IR: ");
    XBee.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
    
}
