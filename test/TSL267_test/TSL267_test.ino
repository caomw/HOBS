// This is the test file for TSL267 sensor
// For comparison purpose, this requires two sensors to be hooked up with analog pin 4, 5
// the values were returned to serials
//   Based on http://arduino.cc/en/Reference/analogRead
// created: 03/23/2014

#undef TWO_SENSOR

int analogPin1 = 5;

#ifdef TWO_SENSOR
int analogPin2 = 5;     
#endif

void setup()
{
  Serial.begin(9600);          //  setup serial
}

int val1, val2;

void loop()
{
  val1 = analogRead(analogPin1);    // read the input pin
  
  #ifdef TWO_SENSOR
  val2 = analogRead(analogPin2);    // read the input pin
  #endif
  
  Serial.print(val1);
  
  #ifdef TWO_SENSOR
  Serial.print(',');
  Serial.println(val2);             // debug value
  #else
  Serial.println();             // a new line
  #endif
  
  delay(10);
}

