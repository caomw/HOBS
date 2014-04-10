// This is the test file for TSL267 sensor
// For comparison purpose, this requires two sensors to be hooked up with analog pin 4, 5
// the values were returned to serials
//   Based on http://arduino.cc/en/Reference/analogRead
// created: 04/10/2014

int analogPin[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};

void setup()
{
  Serial.begin(9600);          //  setup serial
}

void loop()
{
  for (int i = 0; i < 9; ++i) {
    Serial.print(i);
    Serial.print(":");  
    Serial.println(analogRead(analogPin[i]));    // read the input pin
  }  
}

