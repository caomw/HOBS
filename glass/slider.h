#ifndef slider_h
#define slider_h

#define gsIDLE 0
#define gsPRESS 1
#define gsRELEASE 2
#define gsPENDING 3

#define SOFTPOT_THREASHOLD 950
#define SOFTPOT_DELTA_THRESHOLD 20

#define HOLD_THRESHOLD 300   //determine tap vs hold
#define DTAP_THRESHOLD 600  //two taps within this duration -> dtap
#define ACCIDENT_THRESHOLD 300

unsigned long start_time = 0;
unsigned long release_time = 0;
unsigned long last_release_time = 0;

int sliderState = gsIDLE;

int softpotReading = 0;
int softpotPin = A0; //analog pin 0

//error handling
boolean checkDTap = false;
boolean checkRelease = false;

typedef enum {
  gNONE,
  gHOVER,
  gTAP,
  gD_TAP,
  gRELEASE,
  gHOVERCHANGE,
  gPRESS
} gesture_t;

// gesture_t g = gNONE;

gesture_t sliderEvent(int* sDelta, int* sVal) {
  softpotReading = analogRead(softpotPin);
  // int softpotReading = *pot;
  // DEBUG_PRINTLN(softpotReading);

  if(checkRelease) {
    //if pressed again in short duration -> accident
    //else (timeout) -> return release
    if(millis() - release_time > ACCIDENT_THRESHOLD) {
      //it's a real release
      last_release_time = release_time;
      start_time = 0;
      release_time = 0;
      *sVal = 0;
      *sDelta = 0;
      checkRelease = false;
      sliderState = gsRELEASE;
      return gRELEASE;

    } else {
      if(softpotReading <= SOFTPOT_THREASHOLD) {
        //withn duration but have been pressed -> accident
        DEBUG_PRINTLN("catch accidental release");
        checkRelease = false;
        sliderState = gsPRESS;
        release_time = last_release_time;
        return gHOVER;

      } else {
        //within duration but still unpressed -> keep waiting
        
        return gHOVER;
      }
    }

    
  }


  if(softpotReading > SOFTPOT_THREASHOLD ) { // currently not pressed
    if(sliderState == gsPRESS) {
      // previously pressed -> look at duration
      // sliderState = gsRELEASE;
      release_time = millis();
      DEBUG_PRINT("pressed duration: ");
      DEBUG_PRINT(release_time - start_time);
      
      DEBUG_PRINTLN();
      if(release_time - start_time < HOLD_THRESHOLD) {
        //a tap -> check for double tap
        DEBUG_PRINT("previous release duration: ");
        DEBUG_PRINT(release_time - last_release_time);
        DEBUG_PRINTLN();
        if(release_time - last_release_time < DTAP_THRESHOLD) {

          last_release_time = 0;
          start_time = 0;
          release_time = 0;
          *sVal = 0;
          *sDelta = 0;
          sliderState = gsRELEASE;
          return gD_TAP;
        } else {
          last_release_time = release_time;
          start_time = 0;
          release_time = 0;
          *sVal = 0;
          *sDelta = 0;
          sliderState = gsRELEASE;
          return gTAP;
        }

        
      } else {
        //relase from hover (long hold)

        //check if it's accidental
        //p.s. release from hove can't trigger double tap
        checkRelease = true;
        // sliderState = gsPENDING;
        return gHOVER;


        // last_release_time = 0;
        // start_time = 0;
        // release_time = 0;
        // *sVal = 0;
        // *sDelta = 0;
        // return gRELEASE;
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
      if(abs(*sDelta) > SOFTPOT_DELTA_THRESHOLD){ //avoid noise
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

void printGesture(int g, int *delta, int *val){
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
      DEBUG_PRINT(*delta);
      DEBUG_PRINT(" -> ");
      DEBUG_PRINTLN(*val);
      break;
    case 6:
      DEBUG_PRINT("PRESS: ");
      DEBUG_PRINTLN(*val);
      break;
    default:
      DEBUG_PRINTLN("ERR");
      break;
  }
}

#endif