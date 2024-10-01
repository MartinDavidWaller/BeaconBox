/*
 *  Progress.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

 #include "LEDChain.h"
 #include "Progress.h"

 void progressSetUp() {

    ledSetIndexColour(PROGRESS_STATE_0_LED, CRGB::Red);
    ledSetIndexColour(PROGRESS_STATE_1_LED, CRGB::Red);
    ledSetIndexColour(PROGRESS_STATE_2_LED, CRGB::Red);
    ledSetIndexColour(PROGRESS_STATE_3_LED, CRGB::Red);
    ledSetIndexColour(PROGRESS_STATE_4_LED, CRGB::Red);
}

void progressAccessPointOpen() {

  ledSetIndexColour(PROGRESS_STATE_0_LED, CRGB::Green);
}
