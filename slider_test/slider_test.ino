
#define DEBUG
#include "slider.h"

#define gsIDLE 0
#define gsPRESS 1
#define gsRELEASE 2

int softpotReading = 0;
int softpotInitV = 0;
int softpotPin = A0; //analog pin 0
int ledPin = 13;


unsigned long start_time = 0;
unsigned long release_time = 0;
unsigned long last_release_time = 0;
int duration_threshold = 300;
int dtap_duration_threshold = 600;
int delta_threshold = 20;
gesture_t g = gNONE;

int sliderVal = 0;
int sliderDelta = 0;
int sliderState = 0;


void setup()
{
  Serial.begin(9600);
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  // XBee.begin(9600);
  // XBeePacketCounter = 0;

  
  delay(300);
  Serial.println("system begins");
}

void loop() {
  
  g = sliderEvent(&sliderDelta, &sliderVal);
  if (g!= 0) {
    printGesture(g);
  }
  
  delay(10);
}

gesture_t sliderEvent(int* sDelta, int* sVal) {
  softpotReading = analogRead(softpotPin);
  // DEBUG_PRINTLN(softpotReading);
  if(softpotReading > SOFTPOT_THREASHOLD ) { // currently not pressed
    if(sliderState == gsPRESS) {
      // previously pressed -> look at duration
      sliderState = gsRELEASE;
      release_time = millis();
      DEBUG_PRINT("pressed duration: ");
      DEBUG_PRINT(release_time - start_time);
      
      DEBUG_PRINTLN();
      if(release_time - start_time < duration_threshold) {
        //a tap -> check for double tap
        DEBUG_PRINT("previous release duration: ");
        DEBUG_PRINT(release_time - last_release_time);
        DEBUG_PRINTLN();
        if(release_time - last_release_time < dtap_duration_threshold) {

          last_release_time = 0;
          start_time = 0;
          release_time = 0;
          *sVal = 0;
          *sDelta = 0;
          return gD_TAP;
        } else {
          last_release_time = release_time;
          start_time = 0;
          release_time = 0;
          *sVal = 0;
          *sDelta = 0;
          return gTAP;
        }

        
      } else {
        //relase from hover 
        last_release_time = 0;
        start_time = 0;
        release_time = 0;
        *sVal = 0;
        *sDelta = 0;
        return gRELEASE;
      }

      
    } 
    else {
      // has been idle
      sliderState = gsIDLE;
      return gNONE;
    }
  } 
  else {  // slider is being pressed
    
    if(sliderState == gsPRESS) {
      // continuously pressing
      *sDelta = softpotReading - *sVal;
      if(abs(*sDelta) > delta_threshold){ //avoid noise
        *sVal = softpotReading;
        
        return gHOVERCHANGE;
      } else {
        
        return gHOVER;    
      }
      
    } 
    else {
      // first time pressed
      
      sliderState = gsPRESS;
      start_time = millis();
      *sVal = softpotReading;
      *sDelta = 0;
      return gPRESS;
    }
  }
}

void printGesture(int g){
  // DEBUG_PRINT("---");
  switch(g){
    case 0:
      DEBUG_PRINTLN("*none");
      break;
    case 1:
      // DEBUG_PRINT("HOVER: ");
      // DEBUG_PRINTLN(sliderVal);
      break;
    case 2:
      DEBUG_PRINTLN("TAP");
      break;
    case 3:
      DEBUG_PRINTLN("DUBBLE TAP");
      break;
    case 4:
      DEBUG_PRINTLN("RELEASE");
      break;
    case 5:
      DEBUG_PRINT("HOVER CHANGE: ");
      DEBUG_PRINT(sliderDelta);
      DEBUG_PRINT(" -> ");
      DEBUG_PRINTLN(sliderVal);
      break;
    case 6:
      DEBUG_PRINT("PRESS: ");
      DEBUG_PRINTLN(sliderVal);
      break;
    default:
      DEBUG_PRINTLN("ERR");
      break;
  }
}

