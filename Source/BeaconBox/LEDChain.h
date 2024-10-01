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

// Define routines and functions

void ledChainSetUp();
void ledChainBlinkAll();
