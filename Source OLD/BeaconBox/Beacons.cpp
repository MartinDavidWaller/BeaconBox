/*
 *  Beacons.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// Beacon schedule from:
//
// https://iaruhfbeacons.wordpress.com/schedule-of-ibp-ncdxf-beacon-transmissions/
//

#include "Arduino.h"
#include <mutex>
#include <time.h>
#include "BeaconBox.h"
#include "Beacons.h"
#include "Configuration.h"
#include "FormatHelper.h"
#include "LEDChain.h"
#include "StringHelper.h"
#include "SunData.h"
#include "WebServer.h"

#define MAX_NUM_ELEMENTS 10
#define FREQUENCY_DELTA 10
#define MAXIMUM_SPOT_COUNT 5

#define CALLSIGN_MAX_LENGTH 20

extern struct Configuration configuration;

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

void beaconsSpotted(char *spotter, char *spotted, char* rbnType, double frequency) {

  // Serial.printf("beaconsSpotted %-10s, spotted=%-10s, type=%s, frequency %f\n", spotter, spotted, rbnType, frequency);

  // Before searching for each one we can check the type, it should be NCDXF
  // If this a beacon of interest?

if (0 == strcmp(rbnType,"NCDXF")) {
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
  { F_14100, LED_BEACON_FREQUENCY_14100 },
  { F_18110, LED_BEACON_FREQUENCY_18110 },
  { F_21150, LED_BEACON_FREQUENCY_21150 },
  { F_24930, LED_BEACON_FREQUENCY_24930 },
  { F_28200, LED_BEACON_FREQUENCY_28200 }
};

int beaconLEDs[] = {
  LED_4S7B,
  LED_4U1UN,
  LED_4X6TU,
  LED_5Z4B,
  LED_CS3B,
  LED_JA2IGY,
  LED_KH6RS,
  LED_LU4AA,
  LED_OA4B,
  LED_OH2B,
  LED_RR90,
  LED_VE8AT,
  LED_VK6RBP,
  LED_VR2B,
  LED_W6WX,
  LED_YV5B,
  LED_ZL6B,
  LED_ZS6DN
};
  
int activehhh = 0;

void beaconsStepBeacon() {

  xSemaphoreTake(mutex, portMAX_DELAY);
  
  Serial.printf("beaconsStepBeacon ****** %f\n", freqencies[hhh[activehhh].Band]);
  
  // Here we display the active band and then move onto the next

  // Turn off all frequency and beacons leds both on the board and
  // on the webpage.
  
  ledTurnOffAllFrequencyLeds();
  ledTurnOffAllBeaconLeds();

  sendAllFrequenciesOffToBeaconListeners();
  sendAllBeaconsOffToBeaconListeners();

  // Now we need to loop though the spots associated with the band to see
  // if we have anthing.

  // Set the frequency led red to indicate no beacons spotted
  
  CRGB frequencyLEDColour = CRGB::Red;

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

 Serial.printf("........ %-10s %s\n", &spot->Spotter[0], FormatTimeAsDateTime(spot->TimeHeard));

        time_t timeNow;
        time(&timeNow);
         //Serial.printf("........ %s %s\n", FormatTimeAsDateTime(timeNow), FormatTimeAsDateTime(spot->TimeHeard + 60 * 5));
         
        if (timeNow < spot->TimeHeard + 60 * configuration.SpotterTimeOutMinutes) {

          //Serial.printf(".... Beacon %s %f\n", beaconNames[b], freqencies[hhh[activehhh].Band]);
//Serial.printf("........ Yes\n");

          lightBeacon = true;
          break;
        }
      
        

        if (band->Front == band->Rear)
          break;

      } while (cbi != band->Rear);

      // Do we need to light the beacon?
      
      if (true == lightBeacon) {

        // Yes we do.
        
        Serial.printf(".... Beacon %s %f\n", beaconNames[b], freqencies[hhh[activehhh].Band]);
        ledSetIndexColour(beaconLEDs[b],CRGB::Green);

        sendBeaconOnOffToBeaconListeners(beaconNames[b], true);

        // Set the band LED colour green to mark beacons have been seen
        
        frequencyLEDColour = CRGB::Green;
      }
    }
  }

  // Set the frequency LED 
  
  ledSetIndexColour(hhh[activehhh].BandLED,frequencyLEDColour);

  sendFrequencyActiveInActiveToBeaconListeners(freqencies[hhh[activehhh].Band], CRGB::Green == frequencyLEDColour);
  
  // Step on to the next band

  activehhh++;
  if (activehhh > 4) {
    activehhh = 0;
  }

  // Clear the mutex access
  
  xSemaphoreGive(mutex); 
}

// All code below this point implements the OPERATION_MODE_NCDXF_IARU mode of
// operation.

CRGB beaconFrequencyColours[] = 
{
  CRGB( 0x80,0x00,0x00),   // 14.100 Maroon
  
  CRGB( 0x80,0x80,0x00),   // 18.110 Olive **********
  CRGB( 0x00,0x00,0x75),   // 21.150 Navy
  
  //CRGB( 0xff,0xe1,0x19),   // 24.930 Yellow *********
  //CRGB( 0x9a,0x63,0x24),   // 24.930 Brown *********
  //CRGB( 0x49,0x99,0x90),   // 24.930 Teal ********* Goer
  //CRGB( 0xf5,0x82,0x31),   // 24.930 Orange
  //CRGB( 0xff, 0x00, 0x00),   // 14.100 Red
  //CRGB( 0xf0, 0x32, 0xe6),   // 14.100 Magenta
  CRGB( 0xff, 0x14, 0x93),   // 14.100 Deeppink
  //CRGB( 0x91,0x1e,0xb4),   // 24.930 Purple
  //CRGB( 0xa9,0xa9,0xa9),   // 24.930 Grey *********
  //CRGB( 0xff,0xff,0xff)    // 28.200 White
  //CRGB( 0x3c,0xb4,0x4b)    // 28.200 Green
  //CRGB( 0xfa,0xbe,0xd4)    // 28.200 Pink
  CRGB( 0xff,0xd8,0xb1)    // 28.200 Apricot
};

struct beaconFreequencyTime {
  char *Country;
  char *Call;
  int FrequencTimes[5];
  int Beacon;
  double Latitude;
  double Longitude;
} beaconFreequencyTimes[] = {

//
//  Country               Call      Frequency / Time
//                                  14.100        18.110        21.150        24.930        28.200        Beacon    Latitude            Longitude
  { "United Nations NY", CS_4U1UN,  00 * 60 + 00, 00 * 60 + 10, 00 * 60 + 20, 00 * 60 + 30, 00 * 60 + 40, B_4U1UN,   40.7708333333333,   -73.9583333333333 },
  { "Northern Canada",   CS_VE8AT,  00 * 60 + 10, 00 * 60 + 20, 00 * 60 + 30, 00 * 60 + 40, 00 * 60 + 50, B_VE8AT,   68.3125,           -133.458333333333 },
  { "USA (CA)",          CS_W6WX,   00 * 60 + 20, 00 * 60 + 30, 00 * 60 + 40, 00 * 60 + 50, 01 * 60 + 00, B_W6WX,    37.1458333333333,  -121.875 },  
  { "Hawaii",            CS_KH6RS,  00 * 60 + 30, 00 * 60 + 40, 00 * 60 + 50, 01 * 60 + 00, 01 * 60 + 10, B_KH6RS,   20.7708333333333,  -156.375 },
  { "New Zealand",       CS_ZL6B,   00 * 60 + 40, 00 * 60 + 50, 01 * 60 + 00, 01 * 60 + 10, 01 * 60 + 20, B_ZL6B ,  -41.0625,            175.625},  
  { "West Australia",    CS_VK6RBP, 00 * 60 + 50, 01 * 60 + 00, 01 * 60 + 10, 01 * 60 + 20, 01 * 60 + 30, B_VK6RBP, -32.1041666666667,   116.041666666667 },  
  { "Japan",             CS_JA2IGY, 01 * 60 + 00, 01 * 60 + 10, 01 * 60 + 20, 01 * 60 + 30, 01 * 60 + 40, B_JA2IGY,  34.4375,            136.791666666667 },  
  { "Siberia",           CS_RR90,   01 * 60 + 10, 01 * 60 + 20, 01 * 60 + 30, 01 * 60 + 40, 01 * 60 + 50, B_RR90,    54.9791666666667,    82.875 },    
  { "Hong Kong",         CS_VR2B,   01 * 60 + 20, 01 * 60 + 30, 01 * 60 + 40, 01 * 60 + 50, 02 * 60 + 00, B_VR2B,    22.2708333333333,   114.125 },  
  { "Sri Lanka",         CS_4S7B ,  01 * 60 + 30, 01 * 60 + 40, 01 * 60 + 50, 02 * 60 + 00, 02 * 60 + 10, B_4S7B,     6.89583333333333,   79.875 },  
  { "South Africa",      CS_ZS6DN,  01 * 60 + 40, 01 * 60 + 50, 02 * 60 + 00, 02 * 60 + 10, 02 * 60 + 20, B_ZS6DN,  -26.6458333333333,    27.9583333333333 }, 
  { "Kenya",             CS_5Z4B,   01 * 60 + 50, 02 * 60 + 00, 02 * 60 + 10, 02 * 60 + 20, 02 * 60 + 30, B_5Z4B,    -1.27083333333333,   36.625 }, 
  { "Isreal",            CS_4X6TU,  02 * 60 + 00, 02 * 60 + 10, 02 * 60 + 20, 02 * 60 + 30, 02 * 60 + 40, B_4X6TU,   32.0625,             34.7916666666667 },
  { "Finland",           CS_OH2B,   02 * 60 + 10, 02 * 60 + 20, 02 * 60 + 30, 02 * 60 + 40, 02 * 60 + 50, B_OH2B,    60.3125,             24.375 },
  { "Maderia",           CS_CS3B,   02 * 60 + 20, 02 * 60 + 30, 02 * 60 + 40, 02 * 60 + 50, 00 * 60 + 00, B_CS3B,    32.8125,            -17.2083333333333 },
  { "Argentina",         CS_LU4AA,  02 * 60 + 30, 02 * 60 + 40, 02 * 60 + 50, 00 * 60 + 00, 00 * 60 + 10, B_LU4AA,  -34.6041666666667,   -58.375 },
  { "Peru",              CS_OA4B,   02 * 60 + 40, 02 * 60 + 50, 00 * 60 + 00, 00 * 60 + 10, 00 * 60 + 20, B_OA4B,   -12.0625,            -76.9583333333333 },
  { "Venezuela",         CS_YV5B,   02 * 60 + 50, 00 * 60 + 00, 00 * 60 + 10, 00 * 60 + 20, 00 * 60 + 30, B_YV5B,     9.10416666666667,  -67.7916666666667 }
};

void beaconsShowFrequencyColours() {

  ledSetIndexColour(LED_BEACON_FREQUENCY_14100, beaconFrequencyColours[0]);
  ledSetIndexColour(LED_BEACON_FREQUENCY_18110, beaconFrequencyColours[1]);
  ledSetIndexColour(LED_BEACON_FREQUENCY_21150, beaconFrequencyColours[2]);
  ledSetIndexColour(LED_BEACON_FREQUENCY_24930, beaconFrequencyColours[3]);
  ledSetIndexColour(LED_BEACON_FREQUENCY_28200, beaconFrequencyColours[4]);
  FastLED.show();

  char cvtBuffer[100];

  for(int f = 0; f < 5; f++) {
    sprintf(cvtBuffer,"#%02x%02x%02x",beaconFrequencyColours[f].red,beaconFrequencyColours[f].green,beaconFrequencyColours[f].blue); 
    sendFrequencyColourToBeaconListeners(freqencies[f], cvtBuffer);
  }
}

CRGB activeBeaconColour[NUMBER_OF_BEACONS];

void clearActiveBeaconColours() {

  for(int i = 0; i < NUMBER_OF_BEACONS; i++) {
    activeBeaconColour[i] = CRGB::Black;
  }
}

void setBeaconColourAndFrequency(char *beaconCall, int beaconManifest, CRGB requiredColour, char *frequency) {

  // Do we need to change the beacon colour?
  
  if (activeBeaconColour[beaconManifest] != requiredColour) {

    // Yes we do. Convert the colour into HTML text
    
    char cvtBuffer[10];

    sprintf(cvtBuffer,"#%02x%02x%02x",requiredColour.red,requiredColour.green,requiredColour.blue);

    // Send it on to the clients

    sendBeaconColourToBeaconListeners(beaconCall, cvtBuffer, frequency);

    // Save the active colour

    activeBeaconColour[beaconManifest] = requiredColour;
  }
}

void beaconsShowActiveBeacons() {

  // Start by getting the current time

  time_t timeNow;
  time(&timeNow);

  // Turn this into a time info buffer
  
  struct tm *timeInfo;
  timeInfo = localtime(&timeNow);

  char *FormatTimeAsDateTime(time_t _time);

  // Next we need to determine which how far into each 3 minute slot within the
  // hour we are actually in.

  int threeMinutesMinute = timeInfo->tm_min % 3;

  // Now add in the seconds

  int timeSlotSeconds = threeMinutesMinute * 60 + timeInfo->tm_sec;
    
  Serial.printf("%s %d\n",FormatTimeAsDateTime(timeNow), timeSlotSeconds);

  // Loop through all the beacons that we have
  
  for(int b = 0; b < 18; b++) {

    // Set the beacon colour to black

    CRGB beaconColour = CRGB::Black;
    char *formattedFrequency = "\0";
    
    // Loop through all the frequencies that we have
    
    for(int f = 0; f < 5; f++) {

      int frequencyTime = beaconFreequencyTimes[b].FrequencTimes[f];
      //Serial.printf(".... %d\n",frequencyTime);
      if ((timeSlotSeconds >= frequencyTime) && (timeSlotSeconds < frequencyTime + 10)) {

        Serial.printf(".... %f %s\n",freqencies[f],beaconFreequencyTimes[b].Call);

        beaconColour = beaconFrequencyColours[f];

        formattedFrequency = FormatFrequency(freqencies[f]);
       

       break;
        
      }
    }

    ledSetIndexColour(beaconLEDs[beaconFreequencyTimes[b].Beacon], beaconColour);

    if (CRGB::Black == beaconColour) {
      beaconColour = CRGB::White;
    }

    setBeaconColourAndFrequency(beaconFreequencyTimes[b].Call, beaconFreequencyTimes[b].Beacon, beaconColour, formattedFrequency);
  } 
}

void beaconsShowBeaconsInDaylight() {

#if DEBUG_BEACONS_IN_DAYLIGHT > 0
  Serial.printf("beaconsShowBeaconsInDaylight\n");
#endif

  // Start by getting the current time

  time_t timeNow;
  time(&timeNow);

  // Turn this into a time info buffer
  
  struct tm *timeInfo;
  timeInfo = localtime(&timeNow);

  // Loop through all the beacons that we have
  
  for(int b = 0; b < 18; b++) {

#if DEBUG_BEACONS_IN_DAYLIGHT > 0
    Serial.printf("....%s\n",beaconFreequencyTimes[b].Call);
#endif

    // Set the beacon colour to black

    CRGB beaconColour = CRGB::DarkSlateGrey; // CRGB::Brown;

    struct SunData *sunData = GetSunPosition(
      timeInfo->tm_year, 
      timeInfo->tm_mon + 1,
      timeInfo->tm_mday,
      timeInfo->tm_hour,
      timeInfo->tm_min,
      beaconFreequencyTimes[b].Latitude,
      beaconFreequencyTimes[b].Longitude);

    if (sunData->Altitude > 20) {

#if DEBUG_BEACONS_IN_DAYLIGHT > 0
      Serial.printf("....%s %f\n",beaconFreequencyTimes[b].Call, sunData->Altitude);
#endif
      
      beaconColour = CRGB::Yellow;
    }    
    else if (sunData->Altitude > 0) {

#if DEBUG_BEACONS_IN_DAYLIGHT > 0
      Serial.printf("....%s %f\n",beaconFreequencyTimes[b].Call, sunData->Altitude);
#endif
      
      beaconColour = CRGB::Yellow;
      beaconColour.r = beaconColour.r * 0.5;
      beaconColour.g = beaconColour.g * 0.5;
      beaconColour.b = beaconColour.b * 0.5;
    }      

    // Set the LED colour
    
    ledSetIndexColour(beaconLEDs[beaconFreequencyTimes[b].Beacon], beaconColour);

    // Tell any webpage listeners
    
    setBeaconColourAndFrequency(beaconFreequencyTimes[b].Call, beaconFreequencyTimes[b].Beacon, beaconColour, "");
  } 
}
 
 
