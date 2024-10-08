/*
 *  Beacons.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// Mutes https://forum.arduino.cc/t/how-to-use-mutex/964924

// https://github.com/wizard97/Embedded_RingBuf_CPP

#include "Arduino.h"
#include <RingBufCPP.h>
#include "Beacons.h"

//include "freertos/FreeRTOS.h"
//include "freertos/task.h"
//include "freertos/ringbuf.h"

#define MAX_NUM_ELEMENTS 10

#define CALLSIGN_MAX_LENGTH 20

struct beaconCallsignIndex {
  char *Callsign;
  int Index;
};

struct beaconCallsignIndex hhh[] = 
{
  { CS_4S7B,   B_4S7B },
  { CS_4U1UN,  B_4U1UN },
  { CS_4X6TU,  B_4X6TU },
  { CS_5Z4B,   B_5Z4B },
  { CS_CS3B,   B_CS3B },
  { CS_JA2IGY, B_JA2IGY },
  { CS_KH6RS,  B_KH6RS },
  { CS_LU4AA,  B_LU4AA },
  { CS_OA4B,   B_OA4B },
  { CS_OH2B,   B_OH2B },
  { CS_RR90,   B_RR90 },
  { CS_VE8AT,  B_VE8AT },
  { CS_VK6RBP, B_VK6RBP },
  { CS_VR2B,   B_VR2B },
  { CS_W6WX,   B_W6WX },
  { CS_YV5B,   B_YV5B },
  { CS_ZL6B,   B_ZL6B },
  { CS_ZS6DN,  B_ZS6DN }
};

struct beaconHeard {
  char Spotter[CALLSIGN_MAX_LENGTH];
  time_t TimeHeard;
};

struct beaconBand {
  //unsigned long db;
  //unsigned long timestamp;
  //RingbufHandle_t rbuf;
  RingBufCPP<struct beaconHeard, MAX_NUM_ELEMENTS> buf;
};

struct beacon {
   //RingBufCPP<struct beaconHeard, MAX_NUM_ELEMENTS> buf;
   //int x;
   struct beaconBand beaconsBands[NUMBER_OF_FREQUENCIES];
   
};

struct beacon beacons[NUMBER_OF_BEACONS];

void beaconsSetUp() {
  
}

void beaconsSpotted(char *spotter, char *spotted, double frequency) {

  // Serial.printf("beaconsSpotted %-10s, spotted=%-10s, frequency %f\n", spotter, spotted, frequency);

  // If this a beacon of interest?

  for(int i = 0; i < NUMBER_OF_BEACONS; i++) {
    
    if (0 == strcmp(spotted,&hhh[i].Callsign[0])) {

      // Yes this is of interest!

      Serial.printf("beaconsSpotted %-10s, spotted %-10s, frequency %f, index %d\n", spotter, spotted, frequency, hhh[i].Index);
       //Serial.printf("beaconsSpotted ******************* %-10s, index %d\n", spotted, hhh[i].Index );

       break;
    }
  }
}
 
