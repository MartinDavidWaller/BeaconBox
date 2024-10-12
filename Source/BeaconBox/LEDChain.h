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

#define LED_BEACON_FREQUENCY_FIRST LED_BEACON_FREQUENCY_14110
#define LED_BEACON_FREQUENCY_LAST LED_BEACON_FREQUENCY_28200

#define LED_VE8AT 5
#define LED_KH6RS 6
#define LED_W6WX 7
#define LED_4U1UN 8
#define LED_CS3B 9
#define LED_YV5B 10
#define LED_OA4B 11
#define LED_LU4AA 12
#define LED_ZS6DN 13
#define LED_5Z4B 14
#define LED_4X6TU 15
#define LED_OH2B 16
#define LED_RR90 17
#define LED_4S7B 18
#define LED_VR2B 19
#define LED_JA2IGY 20
#define LED_VK6RBP 21
#define LED_ZL6B 22

#define LED_BEACON_FIRST LED_VE8AT
#define LED_BEACON_LAST LED_ZL6B

#define LED_MISC_1 23
#define LED_MISC_2 24

// Define routines and functions

void ledChainSetUp();
void ledChainBlinkAll();
void ledSetIndexColour(int index,CRGB colour);
