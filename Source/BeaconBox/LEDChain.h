/*
 *  LEDChain.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include <FastLED.h>

// The following manifests define the values associated with the LED chain.

#define LED_PIN 14
#define NUM_LEDS 25

#define LED_BEACON_FREQUENCY_14110 0
#define LED_BEACON_FREQUENCY_18110 1
#define LED_BEACON_FREQUENCY_21150 2
#define LED_BEACON_FREQUENCY_24930 3
#define LED_BEACON_FREQUENCY_28200 4

// Define routines and functions

void ledChainSetUp();
void ledChainBlinkAll();
void ledSetIndexColour(int index,CRGB colour);
