/*
 *  LEDChain.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
 #include "LEDChain.h"
 #include "BeaconBox.h"

// Define the working data

CRGB leds[LED_CHAIN_LENGTH];

//
// This method is called to setup the LED chain.
//
void ledChainSetUp() {

#if LED_TYPE == LED_TYPE_WS2812B 

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_CHAIN_LENGTH);
#elif LED_TYPE == LED_TYPE_WS2812D

  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, LED_CHAIN_LENGTH);
#else

  #error LED_TYPE - Unrecognised value.
#endif

  // Set full brightness
  
  FastLED.setBrightness(255);
}

void ledBrightness(uint8_t newBrightness) {
  
  FastLED.setBrightness(newBrightness);
}

//
// This method is called to blink all the LEDs in the chain.
//
void ledChainBlinkAll() {

  //double p = 1.0;
  //for(int i = 0; i < 20; i++) {

    //leds[i] = CRGB::Yellow;

    //leds[i].r = leds[i].r * p;
    //leds[i].g = leds[i].g * p;
    //leds[i].b = leds[i].b * p;

    //p -= 0.05;
  //}

  //FastLED.show();
  //delay(5000);
  
  // Make all the LEDs white
  
  fill_solid(leds, LED_CHAIN_LENGTH, CRGB::White);

  // Show and delay
  
  FastLED.show();
  delay(500);

  // Make all the LEDs black

  fill_solid(leds, LED_CHAIN_LENGTH, CRGB::Black);
  
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

  for(int i = 0; i < LED_CHAIN_LENGTH; i++)
  {
    leds[i] = CRGB::White;

    //for(int j = 0; j < i; j++) {
      //leds[i] = CRGB::White;
    //}
    FastLED.show();
    delay(100);


    
  }

  for(int i = 0; i < LED_CHAIN_LENGTH; i++)
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
