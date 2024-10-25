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

  ledSetIndexColour(LED_BEACON_FREQUENCY_14100, CRGB( 0x80,0x00,0x00));   // Maroon
  ledSetIndexColour(LED_BEACON_FREQUENCY_18110, CRGB( 0x80,0x80,0x00));   // Olive
  ledSetIndexColour(LED_BEACON_FREQUENCY_21150, CRGB( 0x00,0x00,0x75));   // Navy
  ledSetIndexColour(LED_BEACON_FREQUENCY_24930, CRGB( 0xff,0xe1,0x19));   // Yellow
  ledSetIndexColour(LED_BEACON_FREQUENCY_28200, CRGB( 0xff,0xff,0xff));   // White
  FastLED.show();
  delay(2000);
  
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

/*
  // Turn all LEDs on forward
  
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::White;

    FastLED.show();
    delay(100);

    leds[i] = CRGB::Black;
    FastLED.show();
    
  }
  */

/*
  // Turn all LEDs on backwards
  
  for(int i = NUM_LEDS - 1; i > 0; i--)
  {
    leds[i] = CRGB::White;

    FastLED.show();
    delay(100);

    leds[i] = CRGB::Black;
    FastLED.show();
    
  }  
  */

  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::White;

    //for(int j = 0; j < i; j++) {
      //leds[i] = CRGB::White;
    //}
    FastLED.show();
    delay(100);


    
  }

  for(int i = 0; i < NUM_LEDS; i++)
  {


    leds[i] = CRGB::Black;
    FastLED.show();
     delay(100);
    
  }  
   FastLED.show();  
}

void ledSetIndexColour(int index,CRGB colour)
{
  leds[index] = colour;
  FastLED.show();
}

void ledTurnOffAllFrequencyLeds() {

  for(int i = LED_BEACON_FREQUENCY_FIRST; i <= LED_BEACON_FREQUENCY_LAST; i++) {
    ledSetIndexColour(i,CRGB::Black);
  }
}

void ledTurnOffAllBeaconLeds() {

  for(int i = LED_BEACON_FIRST; i <= LED_BEACON_LAST; i++) {
    
    ledSetIndexColour(i,CRGB::Black);
  }
}
