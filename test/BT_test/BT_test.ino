/*
  BT test on Mega

  For old code that works with Aruidno Uno, see git log and pull it from the repo

  Modified upon "Mega multiple serial test"
  
  Receives from the main serial port, sends to the others. 
  Receives from serial1 (BT), sends to the main serial (Serial 0).
 
  This example works only on the Arduino Mega, and the BT command "$$$" is for BlueSMiRF (see https://learn.sparkfun.com/tutorials/using-the-bluesmirf/all). Once the code is running, simply type "D" will yield the following information:

  **Settings***
  BTA=0006666624C3
  BTName=RNBT-24C3
  Baudrt(SW4)=115K
  Mode  =Slav
  Authen=0
  PinCod=1234
  Bonded=0
  Rem=0006666624C4

  created  : 07/25/2013
  modified : 03/26/2013
  author   : Ben Zhang <benzh@eecs.berkeley.edu>
 
 */

#define BT Serial1

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  
  BT.begin(115200);
  
  BT.print("$");  // Print three times individually
  BT.print("$");
  BT.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to

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
