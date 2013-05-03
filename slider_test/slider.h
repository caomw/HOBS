#ifndef slider_h
#define slider_h

#define gsIDLE 0
#define gsPRESS 1
#define gsRELEASE 2

#define SOFTPOT_THREASHOLD 950
#define SOFTPOT_DELTA_THREASHOLD 80
#define DELAY_IN_WAIT 1000000



#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

typedef enum {
  gNONE,
  gHOVER,
  gTAP,
  gD_TAP,
} gesture_t;

#endif