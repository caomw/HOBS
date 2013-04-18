#include <SoftwareSerial.h>
#include <IRremote.h>
#include <assert.h>
#include <TimerOne.h>

// define all the states here
#define IDLE 0
#define INIT 1
#define WAIT 2
#define CONFIRM 3
#define VERIFY 4
#define CONNECTED 5
#define SOFTPOT_THREASHOLD 950
#define DELAY_IN_WAIT 1000000
SoftwareSerial XBee(2, 4); // RX, TX
unsigned long session_id = 0xA90;

// to give it a second thought, the packet should be correctly
// from someone's comment online, XBee samples arrive at a rate of 2.5mSec
// so having a packet smaller than 40 will guarantee collision-free (10 nodes at most)
// I was thinking to have the XBeeMessage in a format that each with specific length
// though this limit the readability and extensibility, but easier to parse

// deviceId   msg     data   checksum
struct XBeePacket {
  char id[2+1];
  char data[4+1];
  char type[1+1];
  char cksum[1+1];
};

// notice, '\0' is not appended
void string_copy(char *dst, const char *src, int start, int end) {
  int i = start;
  while (i <= end) {
    dst[i-start] = src[i++];
  }
  dst[i] = '\0';
}

// to guarantee, dst should be longer than end-start+1
void string_concat(char *dst, const char *src, int pos) {
  int i = 0;
  while (src[i] != '\0') {
    dst[i+pos] = src[i++];
  }
}

// print packet
void printXBeePacket (struct XBeePacket p) {
  Serial.print("[PACKET] id:");
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
int sendXBeePacketFromRaw (const char *id,
			   const char *type,
			   const char *data) {
  char str[20];
  str[0] = '\0';
  string_concat(str, id, 0);
  string_concat(str, type, 2);
  string_concat(str, data, 3);
  string_concat(str, "", 7);
  str[8] = '\0';
  Serial.print("(in sendXBeePacketFromRaw) packet being sent: ");
  Serial.println(str);
  XBee.println(str);
  return 1;
}


// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
int sendXBeePacket (struct XBeePacket p) {
  char str[20];
  str[0] = '\0';
  string_concat(str, p.id, 0);
  string_concat(str, p.type, 2);
  string_concat(str, p.data, 3);
  string_concat(str, p.cksum, 7);
  str[8] = '\0';
  Serial.print("(in sendXBeePacket) packet being sent: ");
  Serial.println(str);
  XBee.println(str);
  return 1;
}


// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
struct XBeePacket readXBeePacket () {
  struct XBeePacket p;
  char strArray[20];
  int i = 0;

  // 'e' indicates error
  string_copy(p.type, "e", 0, 0);
  
  /* if(!XBee.available()) { */
  /*   return p; */
  /* } */
  while (XBee.available()) {
    strArray[i] = XBee.read();
    i++;
  }
  strArray[i] = '\0';
  Serial.print("got packet: ");
  Serial.print(strArray);

  // don't quite understand why i=10 here
  Serial.print(" now i=");
  Serial.println(i);
  
  if (i == 10) {
    string_copy(p.id, strArray, 0, 1);
    string_copy(p.type, strArray, 2, 2);
    string_copy(p.data, strArray, 3, 6);
    string_copy(p.cksum, strArray, 7, 7);
  }

  /* Serial.println("finished parsing packet");  */
  /* printXBeePacket(p); */
  /* Serial.println("Seems to be garbage here");  */
  return p;
}


// An IR LED must be connected to Arduino PWM pin 3.
IRsend irsend;
int softpotPin = A0; //analog pin 0
int sendState = 0;
int ledPin = 8;
int state = 0;
int softpotReading = 0;
struct XBeePacket XBeePacketArr[20];
volatile int XBeePacketCounter;
volatile int selectedXBee;
unsigned long start_time;
unsigned long end_time;

const char segmentDeliminater = ':';

void setup()
{
  Serial.begin(9600);
  state = IDLE;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBee.begin(9600);
  XBeePacketCounter = 0;
}


void loop() {
  switch(state) {
  case IDLE:
    softpotReading = analogRead(softpotPin);
    Serial.print("Softpot Reading: ");
    Serial.println(softpotReading);
    delay(20);
    if (softpotReading < SOFTPOT_THREASHOLD) {
      Serial.println("[INIT] Entering");      
      state = INIT;
    }
    selectedXBee = -1;
    break;
  case INIT:
    Serial.println("[INIT] Sending IR");
    session_id = random(0xFFFF);
    // irsend.sendSony(session_id, 16);
    XBee.print(session_id, HEX);  

    // at the same time listening to any response
    state = WAIT;
    Serial.println("[WAIT] entering state");
    XBeePacketCounter = 0;
    start_time = millis();
    break;
  case WAIT:
    // When entering this state, wait for 1 sec to determine how many signals have been received
    end_time = millis();
    // soft timer expires
    if (end_time - start_time > DELAY_IN_WAIT/1000) {
      // check for the number of replies and act accordingly
      Serial.println(XBeePacketCounter);

      if (XBeePacketCounter == 0) {
	Serial.println("No Msg received");
	state = IDLE;
      }
      else if (XBeePacketCounter == 1) {
	selectedXBee = XBeePacketCounter - 1;
      	Serial.print("[CONFIRM] entering with selected: ");
	Serial.print(selectedXBee);
	Serial.print(" deviceId:");
	Serial.println(XBeePacketArr[selectedXBee].id);
	// send out the selected message
	sendXBeePacketFromRaw(XBeePacketArr[selectedXBee].id, "c", XBeePacketArr[selectedXBee].data);
	state = CONFIRM;
      }
    }
    if(XBee.available()) {
      int i = 0;
      // delay for the complete of transmission
      delay(10);
      Serial.println("[WAIT] Reading Packet");
      struct XBeePacket p = readXBeePacket();
      printXBeePacket(p);

      // now add this packet to packet array if id is new
      for (i = 0; i < XBeePacketCounter; i++) {
	if ( atoi(p.id) == atoi(XBeePacketArr[i].id) ) {
	  break;
	}
      }
      if (i == XBeePacketCounter) {
	// not found, add to the array
	XBeePacketArr[i] =  p;
	XBeePacketCounter++;
      }
    }
    break;
  case CONFIRM:
    // send out the signal to THE selected XBee
    state = CONNECTED;
    break;
  case VERIFY:
    // when there are multiple targets who have responded
    // one of the LED should be on at this case

    // digitalWrite(ledPin, HIGH);
    // then based on the gesture, we start to rotate them 
    state = CONNECTED;
    break;
  case CONNECTED:

    // temporarily for debugging 
    delay(10000);
    state = IDLE;
    
    // start Gesture recognition and communication

    break;
  default:
    break;
  }
}


