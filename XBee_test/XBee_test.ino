#include <SoftwareSerial.h>
#include <IRremote.h>

SoftwareSerial XBee(2, 3); // RX, TX
int ledPin = 13;
int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;
int deviceStatus = 0;
int pendingTimer;
int deviceId = 1;

char XBeeInString[50];

#define statusOff 0
#define statusPending 1
#define statusOn 2


// const unsigned long signalInit = 0xA90;
const unsigned long signalInit = 0xC1AA09F6;
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
  memset(XBeeInString, 0, 50);
  

  readXBeeString(XBeeInString);
  char c = XBeeInString[0];
  if(c != '\0') {
    sendMsg(XBeeInString);  
  }
  
  
  if(Serial.available()) {
    Serial.write(Serial.read());
    XBee.println("msg from serial");
  }
  
  if(irrecv.decode(&results)) {
    
    //todo: filter redundant FFFFFFFF

    Serial.println(results.value, HEX);
    // Serial.println(results.decode_type);
    
    XBee.print(deviceId);
    XBee.print(": ");
    XBee.println(results.value, HEX);

    // switch(results.value) {
    //   case signalInit:
    //     // Serial.println("singalInit");
    //     // XBee.println("singalInit");
    //     sendMsg("signalInit");

    //     break;
      
    //   case signalVerify:
    //     sendMsg("signalVerify");
    //     break;

    //   case signalConfirm:
    //     sendMsg("signalConfirm");
    //     break;

    // }

    irrecv.resume(); // Receive the next value

  }
  
  if(deviceStatus == statusPending) {
    //todo: start blinking
    ;
  }
    
}

void sendMsg(String msg) {
  Serial.print(deviceId);
  Serial.print(": ");
  Serial.print(msg);
  Serial.println();

  XBee.print(deviceId);
  XBee.print(": ");
  XBee.print(msg);
  XBee.println();

}

void readXBeeString (char *strArray) {
  int i = 0;
  if(!XBee.available()) {
    return;
  }
  while (XBee.available()) {
    strArray[i] = XBee.read();
    i++;
  }
  Serial.print("read XBee: ");
  Serial.println(strArray);
  // sendMsg(XBeeInString);
}
