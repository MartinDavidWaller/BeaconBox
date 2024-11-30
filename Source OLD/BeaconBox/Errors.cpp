/*
 *  Errors.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

 #include "LEDChain.h"
 #include "Errors.h"

 void errorsSetUp() {

    //ledSetIndexColour(PROGRESS_STATE_0_LED, CRGB::Red);
    //ledSetIndexColour(PROGRESS_STATE_1_LED, CRGB::Red);
    //ledSetIndexColour(PROGRESS_STATE_2_LED, CRGB::Red);
    //ledSetIndexColour(PROGRESS_STATE_3_LED, CRGB::Red);
    //ledSetIndexColour(PROGRESS_STATE_4_LED, CRGB::Red);
}

void errorOpeningSpiffs() {

  ledSetIndexColour(ERROR_STATE_0_LED, CRGB::Red);
}

void errorOpeningAccessPoint() {

  ledSetIndexColour(ERROR_STATE_1_LED, CRGB::Red);
}

void errorSettingsRequired() {

  ledSetIndexColour(ERROR_STATE_2_LED, CRGB::Red);
}

void errorConnectingToWiFi() {

  ledSetIndexColour(ERROR_STATE_2_LED, CRGB::Red);
  ledSetIndexColour(ERROR_STATE_3_LED, CRGB::Red);
}
