#include <SoftwareSerial.h>
#include <IRremote.h>

SoftwareSerial XBee(2, 3); // RX, TX
int ledPin = 13;
int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;
int deviceStatus = 0;
int pendingTimer;

#define statusOff 0
#define statusPending 1
#define statusOn 2


const unsigned long signalInit = 0xA90;
const unsigned long signalVerify = 0xAF0;
const unsigned long signalConfirm = 0xA70;

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
  if(XBee.available()) {
    Serial.write(XBee.read());
    
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    XBee.write("roger");
  }
  
  if(Serial.available()) {
    Serial.write(Serial.read());
    XBee.println("msg from serial");
  }
  
  if(irrecv.decode(&results)) {
    
    //todo: filter redundant FFFFFFFF

    Serial.println(results.value, HEX);
    // Serial.println(results.decode_type);
    
    XBee.print("IR: ");
    XBee.println(results.value, HEX);

    switch(results.value) {
      case signalInit:
        Serial.println("singalStart");
        XBee.println("singalStart");
        break;
      
      case signalVerify:
        Serial.println("singalVerify");
        XBee.println("singalVerify");
        break;

      case signalConfirm:
        Serial.println("singalConfirm");
        XBee.println("singalConfirm");
        break;

    }

    irrecv.resume(); // Receive the next value

  }
  
  if(deviceStatus == statusPending) {
    //todo: start blinking
    ;
  }
    
}
