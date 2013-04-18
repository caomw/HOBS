#ifndef utils_h
#define utils_h

#include <SoftwareSerial.h>
#include "Arduino.h"

#define DELAY_IN_WAIT 1000000

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

//////////////////////////////////////////////////////////////
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
int sendXBeePacketFromRaw (SoftwareSerial *XBee,
			   const char *id,
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
  (*XBee).println(str);
  return 1;
}


// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
int sendXBeePacket (SoftwareSerial *XBee, struct XBeePacket p) {
  char str[20];
  str[0] = '\0';
  string_concat(str, p.id, 0);
  string_concat(str, p.type, 2);
  string_concat(str, p.data, 3);
  string_concat(str, p.cksum, 7);
  str[8] = '\0';
  Serial.print("(in sendXBeePacket) packet being sent: ");
  Serial.println(str);
  (*XBee).println(str);
  return 1;
}

// define this as a function so that we can flexible change the way we parse packet
// read from the serial port and return the packet
struct XBeePacket readXBeePacket (SoftwareSerial *XBee) {
  struct XBeePacket p;
  char strArray[20];
  int i = 0;

  // 'e' indicates error
  string_copy(p.type, "e", 0, 0);
  
  /* if(!XBee.available()) { */
  /*   return p; */
  /* } */
  while ((*XBee).available()) {
    strArray[i] = (*XBee).read();
    i++;
  }
  strArray[i] = '\0';
  // we appended an '\0', plus an '\n' since it was XBee.println
  Serial.print("got packet, now i=");
  Serial.print(i);
  Serial.print(", and packet: ");
  Serial.print(strArray);
  
  if (i == 10) {
    string_copy(p.id, strArray, 0, 1);
    string_copy(p.type, strArray, 2, 2);
    string_copy(p.data, strArray, 3, 6);
    string_copy(p.cksum, strArray, 7, 7);
  }
  return p;
}

// well, we don't maximize performance, do we?
void digitalToggle(int pin){
  digitalWrite(pin, 1 ^ digitalRead(pin));  
}

#endif
