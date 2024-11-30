/*
 *  Animation.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include "Arduino.h"
#include "Animation.h"
#include "BeaconBox.h"
#include "Configuration.h"

extern struct Configuration configuration;

static void (*modeChangeHandler)(bool manualEvent);
static time_t lastAnimationSwap;
static time_t lastManualEvent;

void animationSetUp(void _modeChangeHandler(bool manualEvent)) {

  // Save the parameters

  modeChangeHandler = _modeChangeHandler;

  // Set any default
  
  lastManualEvent = 0;
  time(&lastAnimationSwap);
}

void animationStart() {

  time(&lastAnimationSwap);
}

void animationAnimate() {

  //Serial.printf("animationAnimate\n");
   
  // Get the time
    
  time_t timeNow;
  time(&timeNow);
    
  // We only need to do something if we are active and we have two, or more,
  // of the animation settings with durations > 0

  int durationsGreaterThanZero = 0;
  
  if (configuration.BeaconsHeardDurationSeconds > 0)
    durationsGreaterThanZero++;
  
  if (configuration.BeaconsActiveDurationSeconds > 0)
    durationsGreaterThanZero++;

  if (configuration.BeaconsInDaylightDurationSeconds > 0)
    durationsGreaterThanZero++;        
          
  if ((true == configuration.AnimationEnabled) && (durationsGreaterThanZero > 1)) {

    // The next decision is based on any last manual event and whether it has
    // times out our not.

    bool proceed = true;

    if (0 != lastManualEvent) {
        
      // We have a manual event outstanding

      if (timeNow > (lastManualEvent + configuration.ManualModeTimeoutSeconds)) {

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

          stepMode = timeNow > (lastAnimationSwap + configuration.BeaconsHeardDurationSeconds);
          break;

        case OPERATION_MODE_NCDXF_IARU:

          stepMode = timeNow > (lastAnimationSwap + configuration.BeaconsActiveDurationSeconds);
          break;

        case OPERATION_MODE_DAYLIGHT:

          stepMode = timeNow > (lastAnimationSwap + configuration.BeaconsInDaylightDurationSeconds);
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
