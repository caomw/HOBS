int controlledPin = 12;
int ledStatePin = 13;

void setup()  
{
  Serial.begin(9600);
  pinMode(controlledPin, OUTPUT);
  Serial.println("system begins!");
  // set the data rate for the SoftwareSerial port
}

void loop()
{
	if(Serial.available()) {
		char inChar = (char)Serial.read(); 
		if(inChar == 't') {
			if(digitalRead(controlledPin)) {
				digitalWrite(controlledPin, LOW);
				digitalWrite(ledStatePin, LOW);
				Serial.println("low!");

			} else {
				digitalWrite(controlledPin, HIGH);
				digitalWrite(ledStatePin, HIGH);
				Serial.println("high!");
			}
		}
		
	}
}