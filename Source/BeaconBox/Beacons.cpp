/*
 *  Beacons.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// Mutes https://forum.arduino.cc/t/how-to-use-mutex/964924

// https://github.com/wizard97/Embedded_RingBuf_CPP

#include "Arduino.h"
#include <mutex>
//include <RingBufCPP.h>
#include <time.h>
#include "Beacons.h"
#include "FormatHelper.h"
#include "LEDChain.h"
#include "StringHelper.h"

//include "freertos/FreeRTOS.h"
//include "freertos/task.h"
//include "freertos/ringbuf.h"

#define MAX_NUM_ELEMENTS 10
#define FREQUENCY_DELTA 10
#define MAXIMUM_SPOT_COUNT 5

#define CALLSIGN_MAX_LENGTH 20

struct beaconCallsignIndex {
  char *Callsign;
  int Index;
};

struct beaconCallsignIndex beaconCallsignIndexes[] = 
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

/*
 * In the spot structure we will store the callsign of the spotter and
 * the time the beacon was heard.
 */
 
struct spot {
  char Spotter[CALLSIGN_MAX_LENGTH];
  time_t TimeHeard;
};

/*
 * In the band structure we will store a circular buffer of spot 
 * structures.
 */
 
struct band {
  int Front = -1;
  int Rear = -1;
  struct spot Spots[MAXIMUM_SPOT_COUNT];
};

/*
 * In the beacon structure we store an array bands. All beacons transmit
 * on each band.
 */
struct beacon {
   struct band Bands[NUMBER_OF_FREQUENCIES];
};

struct beacon beacons[NUMBER_OF_BEACONS];
int spotterWildcardsCount = 0;
char **spotterWildcards;
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

void beaconsSetUp(char *givenSpotterWildcards) {

  Serial.printf("beaconsSetUp %s\n", givenSpotterWildcards);

  // We need to plit up the spotter wilcards on spaces and save them
  // away so we can use them later.

  // Free up any data that we may already have

  if (0 != spotterWildcardsCount) {

    // We have something to free up the array entries

    for(int i = 0; i < spotterWildcardsCount; i++) {
      free(spotterWildcards[i]);
    }

    // Free the array
    
    free(spotterWildcards);
  }
  
  // Split the string up

  char **parts;
  int n = SplitStringOnSpace(givenSpotterWildcards, &parts);
  Serial.printf("%d\n", n);

   // Build the new array

  spotterWildcards = (char**)malloc(n * sizeof(char *));
  
  // For all the parts that we have
  
  for(int i = 0; i < n; i++) {

    Serial.printf("%s\n", parts[i]);

    // Allocate a new buffer and copy the string in

    spotterWildcards[i] = (char*)malloc(strlen(parts[i]) + 1);
    strcpy(spotterWildcards[i], parts[i]);
  }

  // Set the count

  spotterWildcardsCount = n;

  for(int i = 0; i < spotterWildcardsCount; i++) {

    Serial.printf("%s\n", spotterWildcards[i]);
  }  
}

void dequeueSpot(struct band *band) {

  // Check if the queue is empty
  
  if (band->Front == -1) {
    return;
  }
 
    //int data = arr[front];
    //arr[front] = -1;
    
  if (band->Front == band->Rear) {
    band->Front = -1;
    band->Rear = -1;
  }
  else if (band->Front == MAXIMUM_SPOT_COUNT - 1)
    band->Front = 0;
  else
    band->Front++;
 
  return;
}

void enqueueSpot(struct band *band, char *spotter) {

  xSemaphoreTake(mutex, portMAX_DELAY);
  
  // See: https://www.geeksforgeeks.org/introduction-to-circular-queue/

  // Check to see if the queue is full

  if ((band->Front == 0 && band->Rear == MAXIMUM_SPOT_COUNT - 1) ||
            ((band->Rear + 1) % MAXIMUM_SPOT_COUNT == band->Front))
    {
        Serial.printf("enqueueSpot queue full\n");

        dequeueSpot(band);
    }
    
  // Update the rear index

  band->Rear++;
  if (MAXIMUM_SPOT_COUNT == band->Rear) {
    band->Rear = 0;
  }

  // Update the front index

  if (-1 == band->Front) {
    band->Front = 0;
  }

  // OK, make the entry

  struct spot *spot = &band->Spots[band->Rear];

  strcpy(&spot->Spotter[0],spotter);
  time(&spot->TimeHeard);

  Serial.printf("enqueueSpot %-10s, %s\n", &spot->Spotter[0], FormatTimeAsDateTime(spot->TimeHeard));

  xSemaphoreGive(mutex);
}

void beaconsSpotted(char *spotter, char *spotted, double frequency) {

  // Serial.printf("beaconsSpotted %-10s, spotted=%-10s, frequency %f\n", spotter, spotted, frequency);

  // If this a beacon of interest?

  for(int i = 0; i < NUMBER_OF_BEACONS; i++) {

    // Is this a beacon of interest?
    
    if (0 == strcmp(spotted,&beaconCallsignIndexes[i].Callsign[0])) {

      // Yes this is of interest! Next, before we bother with it we need to ensure that 
      // the spotter has a callsign of interest.

      // The spotter may end with a -#, we need to remove this if it is there.

      char *foundAtPointer = strstr(spotter,"-#");
      if (NULL != foundAtPointer) {
        *foundAtPointer = '\0';
      }

      // Check the wilcards
      
      bool wildcardMatch = false;

      for(int i = 0; (i < spotterWildcardsCount) && (false == wildcardMatch); i++) {

        if (wildcmp(spotterWildcards[i], spotter) > 0) {

          // Yes it is a match

          wildcardMatch = true;
        }
      }  

      // Do we have a match?
      
      if (true == wildcardMatch) {

        Serial.printf("beaconsSpotted %-10s, spotted %-10s, frequency %f, index %d\n", spotter, spotted, frequency, beaconCallsignIndexes[i].Index);

        // Yes, we now know that we have a beacon of interest spotted by a callsign of interest.
        // We need to save this fact away. Before we get too involved as need to clean up the
        // frequency as they don't always arrive spot-on.

        int frequencyManifest = -1;
        if (frequency < 14100 + FREQUENCY_DELTA) 
          frequencyManifest = F_14100;
        else if (frequency < 18110 + FREQUENCY_DELTA) 
          frequencyManifest = F_18110;
        else if (frequency < 21150 + FREQUENCY_DELTA) 
          frequencyManifest = F_21150;
        else if (frequency < 24930 + FREQUENCY_DELTA) 
          frequencyManifest = F_24930;
        else if (frequency < 28200 + FREQUENCY_DELTA) 
          frequencyManifest = F_28200;

        // Make sure that we have a valid frequency

        if (-1 == frequencyManifest) {
          Serial.printf("beaconsSpotted frequency %f not matched!\n", frequency);

          return;
        }

        // OK, we now need to add the spot to the circular buffer

        struct band *band = &beacons[beaconCallsignIndexes[i].Index].Bands[frequencyManifest];
        Serial.printf("front %d, rear %d\n", band->Front, band->Rear);

        enqueueSpot(band,spotter);
      }

      //Serial.printf("beaconsSpotted %-10s, spotted %-10s, frequency %f, index %d\n", spotter, spotted, frequency, hhh[i].Index);
       //Serial.printf("beaconsSpotted ******************* %-10s, index %d\n", spotted, hhh[i].Index );

       break;
    }
  }
}

char *beaconNames[] = {
  CS_4S7B,
  CS_4U1UN,
  CS_4X6TU,
  CS_5Z4B,
  CS_CS3B,
  CS_JA2IGY,
  CS_KH6RS,
  CS_LU4AA,
  CS_OA4B,
  CS_OH2B,
  CS_RR90,
  CS_VE8AT,
  CS_VK6RBP,
  CS_VR2B,
  CS_W6WX,
  CS_YV5B,
  CS_ZL6B,
  CS_ZS6DN,
  };

double freqencies[] = {
  14.100,
  18.110,
  21.150,
  24.930,
  28.200
};

void dumpBeacons() {

  xSemaphoreTake(mutex, portMAX_DELAY);
  
  Serial.printf("dumpBeacons ******\n");

  // For all the beacons we have

  for(int b = B_FIRST; b <= B_LAST; b++) {

    // For all the frequencies

    for(int f = F_FIRST; f <= F_LAST; f++) {

      // Pull out the band object

      struct band *band = &beacons[b].Bands[f];

      // Do we have any entries in the circular buffer

      if ((-1 != band->Front) && (-1 != band->Rear)) {

        // Ok, we have entries

        Serial.printf("%-10s %f Front %d, Rear %d\n", beaconNames[b], freqencies[f], band->Front, band->Rear);

        int cbi = -1;
    
        do {

          if (-1 == cbi)
            cbi = band->Front;
          else
            cbi = (cbi + 1) % MAXIMUM_SPOT_COUNT;

          // We know that we can display the active entry

          struct spot* spot = &band->Spots[cbi];

          Serial.printf("........ %-10s %s\n", &spot->Spotter[0], FormatTimeAsDateTime(spot->TimeHeard));

          if (band->Front == band->Rear)
            break;

        } while (cbi != band->Rear);
      }
    }
  }

  xSemaphoreGive(mutex); 
}

struct hhh {

  int Band;
  int BandLED;
  
} hhh[] = 
{
  { F_14100, LED_BEACON_FREQUENCY_14110 },
  { F_18110, LED_BEACON_FREQUENCY_18110 },
  { F_21150, LED_BEACON_FREQUENCY_21150 },
  { F_24930, LED_BEACON_FREQUENCY_24930 },
  { F_28200, LED_BEACON_FREQUENCY_28200 }
};

int beaconLEDs[] = {
  LED_VE8AT,
  LED_KH6RS,
  LED_W6WX,
  LED_4U1UN,
  LED_CS3B,
  LED_YV5B,
  LED_OA4B,
  LED_LU4AA,
  LED_ZS6DN,
  LED_5Z4B,
  LED_4X6TU,
  LED_OH2B,
  LED_RR90,
  LED_4S7B,
  LED_VR2B,
  LED_JA2IGY,
  LED_VK6RBP,
  LED_ZL6B
};
  
int activehhh = 0;

void beaconsStepBeacon() {

  xSemaphoreTake(mutex, portMAX_DELAY);
  
  Serial.printf("beaconsStepBeacon ******\n");
  
  // Here we display the active band and then move onto the next

  // Turn all the band leds off

  for(int i = LED_BEACON_FREQUENCY_FIRST; i <= LED_BEACON_FREQUENCY_LAST; i++) {
    ledSetIndexColour(i,CRGB::Black);
  }

  // Turn all the beacons leds off

  for(int i = LED_BEACON_FIRST; i <= LED_BEACON_LAST; i++) {
    ledSetIndexColour(i,CRGB::Black);
  }

  // Turn the band led on

  ledSetIndexColour(hhh[activehhh].BandLED,CRGB::White);

  // Now we need to loop though the spots associated with the band to see
  // if we have anthing.

  // Loop through all the beacons

  for(int b = 0; b < NUMBER_OF_BEACONS; b++) {

    //Serial.printf(".... Beacon %s %f\n", beaconNames[b], freqencies[hhh[activehhh].Band]);
    
    // Pull out the band that we are interested in
    
    struct band *band = &beacons[b].Bands[hhh[activehhh].Band];

    // Do we have any recent spots?
    
    if ((-1 != band->Front) && (-1 != band->Rear)) {

      // Ok, we have entries

      bool lightBeacon = false;
        

      int cbi = -1;
    
      do {

        if (-1 == cbi)
          cbi = band->Front;
        else
          cbi = (cbi + 1) % MAXIMUM_SPOT_COUNT;

        // We know that we can display the active entry

        struct spot* spot = &band->Spots[cbi];

        time_t timeNow;
        time(&timeNow);
        if (timeNow < spot->TimeHeard + 60 * 5) {

          lightBeacon = true;
          break;
        }
      
        

        if (band->Front == band->Rear)
          break;

      } while (cbi != band->Rear);

          if (true == lightBeacon) {

      Serial.printf(".... Beacon %s %f\n", beaconNames[b], freqencies[hhh[activehhh].Band]);
      ledSetIndexColour(beaconLEDs[b],CRGB::White);
      
    }
    }


  }
  
  // Step on to the next band

  activehhh++;
  if (activehhh > 4) {
    activehhh = 0;
  }

  
  xSemaphoreGive(mutex); 
}
 
