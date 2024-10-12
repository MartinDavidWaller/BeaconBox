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
     FastLED.setBrightness(64);
}

//
// This method is called to blink all the LEDs in the chain.
//
void ledChainBlinkAll() {

  // Make all the LEDs white
  
  fill_solid(leds, NUM_LEDS, CRGB::White);

  // Show and delay
  
  FastLED.show();
  delay(500);

  // Make all the LEDs black

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Show and delay
  
  FastLED.show();
  delay(500);
}

void ledSetIndexColour(int index,CRGB colour)
{
  leds[index] = colour;
  FastLED.show();
}
