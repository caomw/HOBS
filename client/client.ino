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


// deviceId   msg     data   checksum
struct XBeePacket {
  char id[2+1];
  char data[4+1];
  char type[1+1];
  char cksum[1+1];
};


//////////////////////////////////////////////////////////////
// to guarantee, dst should be longer than end-start+1
void string_copy(char *dst, const char *src, int start, int end) {
  int i = start;
  while (i <= end) {
    dst[i-start] = src[i++];
  }
  dst[i] = '\0';
}

// notice, '\0' is not appended
void string_concat(char *dst, const char *src, int pos) {
  int i = 0;
  while (src[i] != '\0') {
    dst[i+pos] = src[i++];
  }
}

///////////////////////////////////////////////////////////////


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
      int len = readXBeeString(packet);
      
      Serial.print("Packet received: ");
      Serial.println(packet);
      Serial.print("Packet len: ");
      Serial.println(len);

      randomDelay = random(1000);
      delay(randomDelay);

      Serial.println("01:a:0101:0");

      struct XBeePacket p;
      string_copy(p.id, "01", 0, 1);
      string_copy(p.type, "a", 0, 0);
      string_copy(p.data, "0101", 0, 3);
      string_copy(p.cksum, "0", 0, 0);
      printXBeePacket(p);
      sendXBeePacket(p);
      
      // state = PENDING;
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

int readXBeeString (char *strArray) {
  int i = 0;
  if(!XBee.available()) {
    return -1;
  }
  while (XBee.available()) {
    strArray[i] = XBee.read();
    i++;
  }
  strArray[i] = '\0';
  return i;
  
  Serial.print("read XBee: ");
  Serial.println(strArray);
  // sendMsg(XBeeInString);
}


// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
int sendXBeePacket (struct XBeePacket p) {
  char str[20];
  str[0] = '\0';
  Serial.println(str);
  string_concat(str, p.id, 0);
  string_concat(str, p.type, 2);
  string_concat(str, p.data, 3);
  string_concat(str, p.cksum, 7);
  str[8] = '\0';
  Serial.println(str);
  XBee.println(str);
  return 1;
}

// print packet
void printXBeePacket (struct XBeePacket p) {
  Serial.print("id:");
  Serial.print(p.id);
  Serial.print("  type:");
  Serial.print(p.type);
  Serial.print("  data:");
  Serial.print(p.data);
  Serial.print("  cksum:");
  Serial.println(p.cksum);
}

// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
struct XBeePacket readXBeePacket () {
  struct XBeePacket p;
  char strArray[20];
  int i = 0;

  // 'e' indicates error
  string_copy(p.type, "e", 0, 0);
  if(!XBee.available()) {
    return p;
  }
  while (XBee.available()) {
    strArray[i] = XBee.read();
    i++;
  }
  strArray[i] = '\0';
  
  if (i == 8) {
    string_copy(p.id, strArray, 0, 1);
    string_copy(p.type, strArray, 2, 2);
    string_copy(p.data, strArray, 3, 6);
    string_copy(p.cksum, strArray, 7, 7);
  }

  return p;
}

