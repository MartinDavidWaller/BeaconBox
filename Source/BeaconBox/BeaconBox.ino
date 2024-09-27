/*
 *  BeaconBox.ino
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
 #include "Arduino.h"

#include <FastLED.h>

#include "BeaconBox.h"
#include "Configuration.h"
#include "LEDChain.h"

// Define the working data

// ???????????

// This is the setup routine. It all starts here.

void setup() {

  // Set up the serial port so we can generate debug etc.
  
  Serial.begin(115200);\
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  } 
  
  Serial.println("Up and running");

  // Setup the LEDs and put on a short display.

  ledChainSetUp();
  ledChainBlinkAll();
}

void loop() {
  
  // put your main code here, to run repeatedly:

}
