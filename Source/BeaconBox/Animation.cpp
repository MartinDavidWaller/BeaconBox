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
static void (*modeChangeHandler)();
static time_t lastAnimationSwap;
static int modeHeardTimeoutSeconds;
static int modeNcdxfIaruTimeoutSeconds;
static int modeDaylightTimeoutSeconds;

void animationSetUp(void _modeChangeHandler()) {

  // Save the parameters

  modeChangeHandler = _modeChangeHandler;

  // Set any default
  
  animationActive = false;
}

void animationSetState(bool _active, int _modeHeardTimeoutSeconds, int _modeNcdxfIaruTimeoutSeconds, int _modeDaylightTimeoutSeconds) {
  
  animationActive = _active;
  modeHeardTimeoutSeconds = _modeHeardTimeoutSeconds;
  modeNcdxfIaruTimeoutSeconds = _modeNcdxfIaruTimeoutSeconds;
  modeDaylightTimeoutSeconds = _modeDaylightTimeoutSeconds;

  time(&lastAnimationSwap);
}

void animationAnimate() {
  
  // We only need to do something if we are active
  
  if (true == animationActive) {

    // Do we need to step the mode?

    bool stepMode = false;
    
    // Get the time
    
    time_t timeNow;
    time(&timeNow);
        
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

      modeChangeHandler();

      // Update the awap time
          
      lastAnimationSwap = timeNow;
    }
  }
}
