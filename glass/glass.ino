#include <SoftwareSerial.h>
#include <IRremote.h>

#define DEBUG
#include "utils.h"

SoftwareSerial XBee(2, 4); // RX, TX

// define all the states here
#define IDLE 0
#define INIT 1
#define WAIT 2
#define CONFIRM 3
#define VERIFY 4
#define CONNECTED 5

#define SOFTPOT_THREASHOLD 950
#define SOFTPOT_DELTA_THREASHOLD 80
#define DELAY_IN_WAIT 1000000

#define gIDLE 0
#define gPRESS 1
#define gRELEASE 2


unsigned long session_id;// = 0xA90;
int clicks;
gesture_t g = gNONE;
// An IR LED must be connected to Arduino PWM pin 3.
IRsend irsend;
int softpotPin = A0; //analog pin 0
int sendState = 0;
int ledPin = 8;
int state = 0;
int softpotReading = 0;
int softpotInitV = 0;
int delta_threshold = 0;
int changes = 0;
struct XBeePacket XBeePacketArr[20];
volatile int XBeePacketCounter;
volatile int selectedXBee;
volatile int initSelectedXBee;
volatile int previousSelected;
unsigned long start_time;
unsigned long last_release_time;

int gState = 0;
int gResolution = 500;
unsigned long gStart_time;
bool gTimerStart = false;

bool released;
bool pressed;
unsigned long end_time;

void setup()
{
  Serial.begin(9600);
  state = IDLE;
  digitalWrite(softpotPin, HIGH); //enable pullup resistor
  randomSeed(analogRead(5));
  digitalWrite(ledPin, LOW);
  XBee.begin(9600);
  XBeePacketCounter = 0;
  pressed = false;
  released = false;
  last_release_time = 0;
  delay(300);
  Serial.print("system begins");
}

int readXBeeString (char *strArray) {
  int i = 0;
  if(!XBee.available()) {
    return -1;
  }
  while (XBee.available()) {
    strArray[i] = XBee.read();
    i++;
  }
  strArray[i] = '\0';
  return i;
}

// should I abstract out the click event and then implement advanced ones?
// well, so the question now is how you detect tap when you have fingers on the slider?
// this is different from click, double click

gesture_t sliderEvent(int *delta, int counts) {
  // all related events are detected here
  gesture_t returnV = gNONE; 
  softpotReading = analogRead(softpotPin);
  DEBUG_PRINT("   gStart_time: ");
  DEBUG_PRINT(gStart_time);
  DEBUG_PRINT("   clicks: ");
  DEBUG_PRINT(clicks);
  DEBUG_PRINT("   gState: ");
  DEBUG_PRINT(gState);
  DEBUG_PRINT("   Softpot Reading: ");
  DEBUG_PRINTLN(softpotReading);

  // gStart_time != 0  => user has pressed, and the detection has started
  // (millis() - gStart_time) > gResolution)  => current time has exceeded the gResolution time from when user pressed, I saw example online which set the gResolution be 300ms, though I use 500 to capture double tap
  if (gStart_time != 0 && (millis() - gStart_time) > gResolution) {
    gState = gIDLE;
    gStart_time = 0;
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
    else
      // if the user has removed the touch, then probably CLICK event
      // might also be D_TAP... bug here
      returnV = gCLICK;
    clicks = 0;
    return returnV;
  }
  
  switch(gState) {
  case gIDLE:
    clicks = 0;
    if (softpotReading < SOFTPOT_THREASHOLD) {
      gState = gPRESS;
      gStart_time = millis();
      softpotInitV = softpotReading;
      return gHOVER;
    }
    break;
  case gPRESS:
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
      gState = gRELEASE;
      clicks++;
    }
    break;
  case gRELEASE:
    if (softpotReading < SOFTPOT_THREASHOLD) {
      gState = gPRESS;
    }
    break;
  default:
    break;
  }
  return gNONE;
}

void loop() {
  // monitor the user input all the time
  // so everytime when loop executes, or at least in some states
  // we will see the results
  if (state == IDLE || state == CONNECTED) {
    g = sliderEvent(&changes, 0);
    if (g!= 0)
      DEBUG_PRINTLN(g);
  }
  delay(10);
  return;
  
  switch(state) {
  case IDLE:
    delay(20);
    if (g == gPRESS) {
      DEBUG_PRINTLN("[INIT] Entering");      
      state = INIT;
      g = gNONE;
    }
    selectedXBee = -1;
    break;
  case INIT:
    session_id = random(0xFFFF);
    DEBUG_PRINT("[INIT] Sending IR: ");
    DEBUG_PRINTLN(session_id);
    irsend.sendSony(session_id, 16);
    // XBee.print(session_id, HEX);  

    // at the same time listening to any response
    state = WAIT;
    DEBUG_PRINTLN("[WAIT] entering state");
    XBeePacketCounter = 0;
    start_time = millis();
    break;
  case WAIT:
    // When entering this state, wait for 1 sec to determine how many signals have been received
    end_time = millis();
    // soft timer expires
    if (end_time - start_time > DELAY_IN_WAIT/1000) {
      // check for the number of replies and act accordingly
      DEBUG_PRINTLN(XBeePacketCounter);

      if (XBeePacketCounter == 0) {
	DEBUG_PRINTLN("No Msg received");
	state = IDLE;
      }
      else if (XBeePacketCounter == 1) {
	selectedXBee = XBeePacketCounter - 1;
      	Serial.print("[CONFIRM] entering with selected: ");
	Serial.print(selectedXBee);
	Serial.print(" deviceId:");
	DEBUG_PRINTLN(XBeePacketArr[selectedXBee].id);
	// send out the selected message
	sendXBeePacketFromRaw(&XBee, XBeePacketArr[selectedXBee].id, "c", XBeePacketArr[selectedXBee].data);
	state = CONFIRM;
      }
      else {
	// there are multiple repliers, need to adjust
	// sort the received Id or actually no need if we have TDMA

	// complex part to implement, take a coffee and then start
	selectedXBee = XBeePacketCounter / 2;
	initSelectedXBee = selectedXBee;
	previousSelected = 0;
	released = false;
	pressed = true;
	state = VERIFY;
      }
    }
    if(XBee.available()) {
      int i = 0;
      // delay for the complete of transmission
      delay(10);
      DEBUG_PRINTLN("[WAIT] Reading Packet");
      struct XBeePacket p = readXBeePacket(&XBee);
      printXBeePacket(p);

      // now add this packet to packet array if id is new
      for (i = 0; i < XBeePacketCounter; i++) {
	if ( atoi(p.id) == atoi(XBeePacketArr[i].id) ) {
	  break;
	}
      }
      if (i == XBeePacketCounter) {
	// not found, add to the array
	XBeePacketArr[i] =  p;
	XBeePacketCounter++;
      }
    }
    break;
  case CONFIRM:
    // send out the signal to THE selected XBee
    // WAIT for the confirmation message to arrive
    // now confirm is almost nothing...
    state = CONNECTED;    
    break;
  case VERIFY:
    // when there are multiple targets who have responded
    // one of the LED should be on at this case
    DEBUG_PRINT("[VERIFY] now trying ");
    DEBUG_PRINT(selectedXBee);
    DEBUG_PRINT(", id=");
    DEBUG_PRINT(XBeePacketArr[selectedXBee].id);
    DEBUG_PRINT(", pressed=");
    DEBUG_PRINT(pressed);
    DEBUG_PRINT(", released=");
    DEBUG_PRINTLN(released);
    if (selectedXBee != previousSelected) {
      sendXBeePacketFromRaw(&XBee, XBeePacketArr[selectedXBee].id, "v", XBeePacketArr[selectedXBee].data);
      previousSelected = selectedXBee;
    }

    // configure the slider dynamically    
    delta_threshold = SOFTPOT_DELTA_THREASHOLD;
    changes = 0;
    if (XBeePacketCounter > 5) {
      // this happens really rare
      // you will only have 800 of the whole slider to use
      // then each step is 800/XBeePacketCounter
      delta_threshold = 800/XBeePacketCounter;
    }
    
    softpotReading = analogRead(softpotPin);
    if (pressed == true && softpotReading < SOFTPOT_THREASHOLD){
      // still in hold
      pressed = true;    
    }
     
    // should detect double click here:
    if (pressed == true && softpotReading > SOFTPOT_THREASHOLD) {
      // hand left, expected to have a tap
      last_release_time = millis();
      pressed = false;
      released = true;
      DEBUG_PRINTLN("released");
    }
    if (released == true && softpotReading < SOFTPOT_THREASHOLD) {
      // detect if tapped again in a timely fashion
      unsigned long new_release_time = millis();
      if (new_release_time - last_release_time < 400) {
	DEBUG_PRINTLN("tap event detected");
	sendXBeePacketFromRaw(&XBee, XBeePacketArr[selectedXBee].id, "c", XBeePacketArr[selectedXBee].data);
	state = CONNECTED;
      }      
    }
    if (released == true && softpotReading > SOFTPOT_THREASHOLD) {
      // timeout happens?      
      unsigned long new_release_time = millis();
      if (new_release_time - last_release_time > 1000) {
	// definitely timeout
	DEBUG_PRINTLN("release timeout");
	state = IDLE;
      }      
    }     
	
    if (pressed == true) {      
      changes = (softpotReading - softpotInitV) / delta_threshold;      
      selectedXBee = initSelectedXBee + changes;
      if (selectedXBee < 0) {
	selectedXBee = 0;
      }
      if (selectedXBee >= XBeePacketCounter) {
	selectedXBee = XBeePacketCounter-1;
      }      
    }
    
    break;
  case CONNECTED:
    // based on gesture, define actions
    if (g == gCLICK) {
      sendXBeePacketFromRaw(&XBee, XBeePacketArr[selectedXBee].id, "i", "0001");
      DEBUG_PRINTLN("[CONNECTED] commands 0001");
      g = gNONE;
    }
    else if (g == gD_TAP) {
      // disconnected
      sendXBeePacketFromRaw(&XBee, XBeePacketArr[selectedXBee].id, "d", XBeePacketArr[selectedXBee].data);
      DEBUG_PRINTLN("[DISCONNECTED] entering IDLE");
      state = IDLE;
      g = gNONE;
    }    
    break;
  default:
    break;
  }
}


