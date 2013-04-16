#include <SoftwareSerial.h>
#include <IRremote.h>

// Trying to elucidate the way communication works first
// current design enforces a simplest design on the receiver side
// no scheduling, purely acknowledge everything it has received with a random delay
#define IDLE 0
#define PENDING 1
#define CONNECTED 2

SoftwareSerial XBee(2, 3); // RX, TX
int ledPin = 13;
int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;
int deviceStatus = 0;
int pendingTimer;
int deviceId = 1;
char XBeeInString[50];
int state = IDLE;
unsigned long randomDelay = 0;

#define statusOff 0
#define statusPending 1
#define statusOn 2

struct XBeeMessage {
  int deviceId;
  unsigned long msg;
};

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

  randomSeed(analogRead(5));
}

int tmpTest = 0;

void loop()
{
  switch(state) {
  case IDLE:
    // purely listening and then react by response
    // for now test use random
    // if(irrecv.decode(&results)) {
    
    if(XBee.available()) {
      delay(5);
      char packet[50];
      int i = 0;
      while(XBee.available()) {
	packet[i++] = XBee.read();
      }
      packet[i] = '\0';
      Serial.print("Packet received: ");
      Serial.println(packet);

      randomDelay = random(1000);
      delay(randomDelay);

      Serial.print(deviceId);
      Serial.print(":");
      Serial.print(packet);
      Serial.println();

      XBee.print(deviceId);
      XBee.print(":");
      XBee.print(packet);
      XBee.println();
    }
    break;
  case PENDING:
    if(XBee.available()) {
      Serial.print(XBee.read());
      return;
      
      // determine if the designated id is the deviceId
      // read XBee message first
      if (1 == deviceId) {
	// have been confirmed
	state = CONNECTED;
      }
      else {
	state = IDLE;
      }
    }
    if(irrecv.decode(&results)) {
      randomDelay = random(100);
      delay(randomDelay);
      XBee.print(deviceId);
      XBee.print(":");
      XBee.println(results.value, HEX);
      state = PENDING;
    }
    break;
  case CONNECTED:
    // pass
    break;
  default:
    break;
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
