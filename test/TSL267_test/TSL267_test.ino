// This is the test file for TSL267 sensor
// For comparison purpose, this requires two sensors to be hooked up with analog pin 4, 5
// the values were returned to serials
//   Based on http://arduino.cc/en/Reference/analogRead
// created: 03/23/2014

int analogPin1 = 4;     
int analogPin2 = 5;     

void setup()
{
  Serial.begin(9600);          //  setup serial
}

int val1, val2;

void loop()
{
  val1 = analogRead(analogPin1);    // read the input pin
  val2 = analogRead(analogPin2);    // read the input pin
  Serial.print(val1);
  Serial.print(',');
  Serial.println(val2);             // debug value
  delay(10);
}

