/*
 *  LEDChain.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
#include "BeaconBox.h"
#include "LEDChain.h"

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

  // ESP32 onboard blue led
  
  //pinMode(2,OUTPUT);
  //digitalWrite(2,HIGH);
}

void ledBrightness(uint8_t newBrightness) {
  
  FastLED.setBrightness(newBrightness);
}

#if BUILD_TYPE == BUILD_TYPE_YAESU_MAP

int ledDisplayOrder[]  = 
{
  LED_BEACON_FREQUENCY_14100,
  LED_BEACON_FREQUENCY_18110,
  LED_BEACON_FREQUENCY_21150,
  LED_BEACON_FREQUENCY_24930,
  LED_BEACON_FREQUENCY_28200,
  LED_DATA,

  LED_KH6RS,
  LED_VE8AT,
  LED_W6WX,
  LED_4U1UN,
  LED_YV5B,
  LED_OA4B,
  LED_LU4AA,
  LED_CS3B,  
  LED_OH2B,
  LED_4X6TU,
  LED_5Z4B,
  LED_ZS6DN,
  LED_RR90,
  LED_4S7B,
  LED_JA2IGY,
  LED_VR2B,
  LED_VK6RBP,
  LED_ZL6B
};

#endif

//
// This method is called to blink all the LEDs in the chain.
//
void ledChainBlinkAll() {
  
  // Make all the LEDs white, show, and delay
  
  fill_solid(leds, LED_CHAIN_LENGTH, CRGB::White);
  FastLED.show();
  delay(500);

  // Make all the LEDs black, show, and delay

  fill_solid(leds, LED_CHAIN_LENGTH, CRGB::Black);
  FastLED.show();
  delay(500);

  // For the complate chain, incrementally turn them all on
  
  for(int i = 0; i < LED_CHAIN_LENGTH - 1; i++)
  { 
    // Make the LED white, show, and delay

#if BUILD_TYPE == BUILD_TYPE_YAESU_MAP
    leds[ledDisplayOrder[i]] = CRGB::White; 
#else
    leds[i] = CRGB::White;
#endif    
  
    FastLED.show();
    delay(100);
  }

  // For the complate chain, incrementally turn them all off
  
  for(int i = 0; i < LED_CHAIN_LENGTH - 1; i++)
  {
    // Make the LED black, show, and delay
    
#if BUILD_TYPE == BUILD_TYPE_YAESU_MAP
    leds[ledDisplayOrder[i]] = CRGB::Black; 
#else
    leds[i] = CRGB::Black;
#endif        
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
