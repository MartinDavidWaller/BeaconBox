/*
 *  Animation.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include "Arduino.h"
#include "Animation.h"
#include "BeaconBox.h"

static bool animationActive;
static void (*modeChangeHandler)(bool manualEvent);
static time_t lastAnimationSwap;
static time_t lastManualEvent;
static int modeHeardTimeoutSeconds;
static int modeNcdxfIaruTimeoutSeconds;
static int modeDaylightTimeoutSeconds;
static int manualEventTimeoutSeconds;

void animationSetUp(void _modeChangeHandler(bool manualEvent)) {

  // Save the parameters

  modeChangeHandler = _modeChangeHandler;

  // Set any default
  
  animationActive = false;
  lastManualEvent = 0;
}

void animationSetState(bool _active, int _modeHeardTimeoutSeconds, int _modeNcdxfIaruTimeoutSeconds, int _modeDaylightTimeoutSeconds, int _manualEventTimeoutSeconds) {
  
  animationActive = _active;
  modeHeardTimeoutSeconds = _modeHeardTimeoutSeconds;
  modeNcdxfIaruTimeoutSeconds = _modeNcdxfIaruTimeoutSeconds;
  modeDaylightTimeoutSeconds = _modeDaylightTimeoutSeconds;
  manualEventTimeoutSeconds = _manualEventTimeoutSeconds;

  time(&lastAnimationSwap);
}

void animationAnimate() {

  //Serial.printf("animationAnimate\n");
   
  // Get the time
    
  time_t timeNow;
  time(&timeNow);
    
  // We only need to do something if we are active

  if (true == animationActive) {

    // The next decision is based on any last manual event and whether it has
    // times out our not.

    bool proceed = true;

    if (0 != lastManualEvent) {

      Serial.printf("...ManualEvent %d\n",manualEventTimeoutSeconds);
        
      // We have a manual event outstanding

      if (timeNow > (lastManualEvent + manualEventTimeoutSeconds)) {

        // Yes, procees

        proceed = true;

        // Clear down the last manual event time

        lastManualEvent = 0;

        // Refresh the lastAnimationSwap time
        
        time(&lastAnimationSwap);
      }
      else {

        // Not time to proceed
        
        proceed = false;
      }
    }

    // Do we procees?

    if (true == proceed) {
      
      // Do we need to step the mode?

      bool stepMode = false;
            
      // We are active! What mode are we in?

      switch(activeMode) {

        case OPERATION_MODE_BEACONS_HEARD:

          stepMode = timeNow > (lastAnimationSwap + modeHeardTimeoutSeconds);
          break;

        case OPERATION_MODE_NCDXF_IARU:

          stepMode = timeNow > (lastAnimationSwap + modeNcdxfIaruTimeoutSeconds);
          break;

        case OPERATION_MODE_DAYLIGHT:

          stepMode = timeNow > (lastAnimationSwap + modeDaylightTimeoutSeconds);
          break;
      }

      // Do we need to step mode?

      if (true == stepMode) {

        // Yes, it's time to move on. Step the mode

        modeChangeHandler(false);

        // Update the awap time
          
        lastAnimationSwap = timeNow;
      }
    }
  }
}

void animationManualEvent() {

  // Update the time of the last manual event

  time(&lastManualEvent);
  Serial.printf("...animationManualEvent %d\n",lastManualEvent);
}
