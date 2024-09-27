/*
 *  LEDChain.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
 #include "LEDChain.h"

// Define the working data

CRGB leds[NUM_LEDS];

//
// This method is called to setup the LED chain.
//
void ledChainSetUp() {

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
}

//
// This method is called to blink all the LEDs in the chain.
//
void ledChainBlinkAll() {

  // There is a better way of doing this but...

  // Make all the LEDs white
  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
  }

  // Show and delay
  
  FastLED.show();
  delay(500);

  // Make all the LEDs black
  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }  
  
  // Show and delay
  
  FastLED.show();
  delay(500);
}
