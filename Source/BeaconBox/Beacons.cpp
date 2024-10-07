/*
 *  Beacons.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// Mutes https://forum.arduino.cc/t/how-to-use-mutex/964924

// https://github.com/wizard97/Embedded_RingBuf_CPP

//include <RingBufCPP.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"

#define MAX_NUM_ELEMENTS 10

struct beaconHeard {
  unsigned long db;
  unsigned long timestamp;
};

struct hhhh {
   //RingBufCPP<struct beaconHeard, MAX_NUM_ELEMENTS> buf;
   RingbufHandle_t rbuf;
};

void beaconsSetUp() {
  
}
