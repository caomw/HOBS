/*
  BT test on Mega

  For old code that works with Aruidno Uno, see git log and pull it from the repo

  Modified upon "Mega multiple serial test"
  
  Receives from the main serial port, sends to the others. 
  Receives from serial port 2 (BT), sends to the main serial (Serial 0).
 
  This example works only on the Arduino Mega
 
  created 07/25/2013
  by Ben Zhang <benzh@eecs.berkeley.edu>
 
 */

#define BT Serial1

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  
  BT.begin(57600);
  BT.println("AT");  // Print three times individually
  
  delay(100);  // Short delay, wait for the Mate to send back CMD

}

String serialString;

void loop() {
  // read from port 1, send to port 0:
  if (BT.available()) {
    int inByte = BT.read();
    Serial.write(inByte); 
  }
  
  // read from port 0, send to port 1:
  if (Serial.available()) {
    delay(50);                 //delay to allow buffer to fill 
    while (Serial.available()) {
      char c = Serial.read();  //gets one byte from serial buffer
      serialString += c; //makes the string readString
    }
    
    Serial.println(serialString);
    BT.println(serialString);
    serialString = "";
    
  }
}
