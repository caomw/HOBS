#ifndef utils_h
#define utils_h

#include <SoftwareSerial.h>
#include "Arduino.h"

// #define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#ifdef DEBUG_TAG
  #define DEBUG_TAGGING(x, y)  Serial.print(x); Serial.print(y)
#else
  #define DEBUG_TAGGING(x, y) 
#endif

#define DELAY_IN_WAIT 1000000

// to give it a second thought, the packet should be correctly
// from someone's comment online, XBee samples arrive at a rate of 2.5mSec
// so having a packet smaller than 40 will guarantee collision-free (10 nodes at most)
// I was thinking to have the XBeeMessage in a format that each with specific length
// though this limit the readability and extensibility, but easier to parse

// deviceId   msg     data   checksum
struct XBeePacket {
  char id[2+1];
  char func[1+1];
  char var[3+1];
  char data[3+1];
};

// func:
// C: control
// S: set
// R: read
// A: ack

//////////////////////////////////////////////////////////////
// notice, '\0' is not appended
void string_copy(char *dst, const char *src, int start, int end) {
  int i = start;
  while (i <= end) {
    dst[i-start] = src[i++];
  }
  dst[i-start] = '\0';
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
  DEBUG_PRINT("[PACKET] id:");
  DEBUG_PRINT(p.id);
  DEBUG_PRINT("  function:");
  DEBUG_PRINT(p.func);
  DEBUG_PRINT("  variable:");
  DEBUG_PRINT(p.var);
  DEBUG_PRINT("  data:");
  DEBUG_PRINTLN(p.data);
}

// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
int sendXBeePacketFromRaw (SoftwareSerial *XBee,
			   const char *id,
			   const char *func,
			   const char *var,
                           const char *data) {
  char str[20];
  str[0] = '\0';
  string_concat(str, id, 0);
  string_concat(str, func, 2);
  string_concat(str, var, 3);
  string_concat(str, data, 6);
  str[9] = '\0';
  DEBUG_PRINT("(in sendXBeePacketFromRaw) packet being sent: ");
  DEBUG_PRINTLN(str);
  (*XBee).println(str);
  return 1;
}


// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
int sendXBeePacket (SoftwareSerial *XBee, struct XBeePacket p) {
  char str[20];
  str[0] = '\0';
  string_concat(str, p.id, 0);
  string_concat(str, p.func, 2);
  string_concat(str, p.var, 3);
  string_concat(str, p.data, 6);
  str[9] = '\0';
  DEBUG_PRINT("(in sendXBeePacket) packet being sent: ");
  DEBUG_PRINTLN(str);
  (*XBee).println(str);
  return 1;
}

// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
struct XBeePacket readXBeePacket (SoftwareSerial *XBee) {
  struct XBeePacket p;
  char strArray[20];
  int i = 0;

  /* if(!XBee.available()) { */
  /*   return p; */
  /* } */
  while ((*XBee).available()) {
    strArray[i] = (*XBee).read();
    i++;
    if(i>20) {
      //abandon the packet
      return p;
    }
  }
  strArray[i] = '\0';
  if (strArray[i-1] == '\n') {
    strArray[i-1] = '\0';
    i--;
  }
  // we appended an '\0', plus an '\n' since it was XBee.println
  // DEBUG_PRINT(i);
  // DEBUG_PRINT(", ");
  // DEBUG_PRINTLN(strArray);

  
  if (i == 10 || i == 9) {
    string_copy(p.id, strArray, 0, 1);
    string_copy(p.func, strArray, 2, 2);
    string_copy(p.var, strArray, 3, 5);
    string_copy(p.data, strArray, 6, 8);
  }
  else {
    // something wrong with the received packet....
    DEBUG_PRINTLN("packet size incorrect");
  }
  return p;
}

// well, we don't maximize performance, do we?
void digitalToggle(int pin){
  digitalWrite(pin, 1 ^ digitalRead(pin));  
}

int readStringfromSerial (SoftwareSerial *SS, char *strArray) {
  int i = 0;
  while ((*SS).available()) {
    strArray[i] = (*SS).read();
    i++;
  }
  strArray[i] = '\0';

  if (strArray[i-1] == '\n') {
    strArray[i-1] = '\0';
  }
  DEBUG_PRINT("read message: ");
  DEBUG_PRINT(strArray);
  DEBUG_PRINT("  count: ");
  DEBUG_PRINTLN(i);
  return i;
}

#endif
