

#define DEBUG
#include "slider.h"

#define gsIDLE 0
#define gsPRESS 1
#define gsRELEASE 2

int softpotReading = 0;
int softpotInitV = 0;
int softpotPin = A0; //analog pin 0
int ledPin = 13;

bool released;
bool pressed;
unsigned long start_time;
unsigned long release_time;
unsigned long last_release_time;
int duration_threshold = 1000;

gesture_t g = gNONE;
int changes = 0;
unsigned long gStart_time;
bool gTimerStart = false;
int gResolution = 300;
int gState = 0;
int clicks = 0;
int delta_threshold = 0;

int sliderState = 0;


void setup()
{
  Serial.begin(9600);
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  // XBee.begin(9600);
  // XBeePacketCounter = 0;
  pressed = false;
  released = false;
  last_release_time = 0;
  delay(300);
  Serial.print("system begins");
}

void loop() {
  // monitor the user input all the time
  // so everytime when loop executes, or at least in some states
  // we will see the results
  
  // g = sliderEvent(&changes, 0);
  g = sliderEvent2();
  if (g!= 0)
  Serial.println(g);
  
  delay(10);
}

gesture_t sliderEvent2() {
  softpotReading = analogRead(softpotPin);


  DEBUG_PRINT("   gStart_time: ");
  DEBUG_PRINT(gStart_time);
  DEBUG_PRINT("   clicks: ");
  DEBUG_PRINT(clicks);
  DEBUG_PRINT("   gState: ");
  DEBUG_PRINT(gState);
  DEBUG_PRINT("   Softpot Reading: ");
  DEBUG_PRINTLN(softpotReading);

  if(softpotReading > SOFTPOT_THREASHOLD ) { // currently not pressed
    if(sliderState == gsPRESS) {
      // previously pressed -> look at duration
      sliderState = gsRELEASE;
      release_time = millis();
      if(release_time - start_time < duration_threshold) {
        //a tap
        last_release_time = release_time;
        start_time = 0;
        release_time = 0;
        return gTAP;
      } else {
        //relase from hover  
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
      return gHOVER;
    } 
    else {
      // first time pressed
      sliderState = gsPRESS;
      start_time = millis();
      return gHOVER;
    }
  }
}



gesture_t sliderEvent(int *delta, int counts) {
  // all related events are detected here
  gesture_t returnV = gNONE; 
  softpotReading = analogRead(softpotPin);

  if(g != 0) {


  DEBUG_PRINT("   gStart_time: ");
  DEBUG_PRINT(gStart_time);
  DEBUG_PRINT("   clicks: ");
  DEBUG_PRINT(clicks);
  DEBUG_PRINT("   gState: ");
  DEBUG_PRINT(gState);
  DEBUG_PRINT("   Softpot Reading: ");
  DEBUG_PRINTLN(softpotReading);

  }
  // gStart_time != 0  => user has pressed, and the detection has started
  // (millis() - gStart_time) > gResolution)  => current time has exceeded the gResolution time from when user pressed, I saw example online which set the gResolution be 300ms, though I use 500 to capture double tap
  if (gStart_time != 0 && (millis() - gStart_time) > gResolution) {
    
    // if now user still holds
    // determine based on click numbers => 0: HOVER; 1:TAP; 2:D_TAP
    if (softpotReading < SOFTPOT_THREASHOLD) {
      if (clicks == 0) {
        returnV = gHOVER;
      }
      else if (clicks == 1) {
        returnV = gTAP;
      }
      else if (clicks == 2) {
        returnV = gD_TAP;
      }
    }
    else {
      // if the user has removed the touch, then probably CLICK event
      // might also be D_TAP... bug here
      //returnV = gCLICK;
    }
  
    gState = gsIDLE;
    gStart_time = 0;
    clicks = 0;
    return returnV;
  }
  
  switch(gState) {
  case gsIDLE:
    clicks = 0;
    if (softpotReading < SOFTPOT_THREASHOLD) {
      gState = gsPRESS;
      gStart_time = millis();
      softpotInitV = softpotReading;
      return gHOVER;
    }
    break;

  case gsPRESS:
    if (softpotReading < SOFTPOT_THREASHOLD) {
      delta_threshold = SOFTPOT_DELTA_THREASHOLD;
      *delta = 0;
      if (counts > 5) {
        // this happens really rare
        // you will only have 800 of the whole slider to use
        // then each step is 800/counts
        delta_threshold = 800/counts;
      }
      *delta = (softpotReading - softpotInitV) / delta_threshold;
    }
    else {
      // only add to click when state goes from PRESS to RELEASE
      gState = gsRELEASE;
      clicks++;
    }
    break;

  case gsRELEASE:
    if (softpotReading < SOFTPOT_THREASHOLD) {
      gState = gsPRESS;
    }
    break;

  default:
    break;
  }
  
  return gNONE;
}