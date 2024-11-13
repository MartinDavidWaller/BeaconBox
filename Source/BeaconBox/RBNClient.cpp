/*
 *  RBNClient.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include "Arduino.h"
#include <WiFi.h>
#include "Beacons.h"
#include "LEDChain.h"
#include "RBNClient.h"
#include "StringHelper.h"

WiFiClient rbnClient;                             // WiFi client
char rbnLineBuffer[RBN_LINE_BUFFER_SIZE + 1];     // Line buffer for the RBN data
void (*spotHandler)(char *spotter, char *spotted, double frequency, char *rbnType, char *rdbTime);

bool rbnClientConnect(char *address, int port, void localSpotHandler(char *spotter, char*spotted, double frequency, char *rbnType, char* rbnTime)) {

  // Save the spot handler

  spotHandler = localSpotHandler;
  
  // Try to make the connection

  rbnClient.connect(address, port);

  // Return the result
  
  return rbnClient.connected();
}

bool rbnClientProcessData(char *callsign) {

  // Build the return value
  
  bool retVal = false;
  
  // Check to see if we are still connected

  if (true == rbnClient.connected()) {

    // Set the return value true, we are still connected

    retVal = true;
    
    // Do we have data avaiable to read?
        
    if (rbnClient.available()) {

      // If the LED is enabled then tuen it on
          
      //if (true == IS_LED_ENABLED((&configuration)))
          //digitalWrite(LED_PIN,HIGH);

      // Yes, clear the buffer and read until the end of line
          
      int bi = 0;
      memset(rbnLineBuffer,0,RBN_LINE_BUFFER_SIZE + 1);
      while ((rbnClient.connected()) && (rbnClient.available()) && (bi < RBN_LINE_BUFFER_SIZE)) {

        //Serial.printf("%s\n",rbnLineBuffer);
            
        // Read the character and save it away
            
        char c = rbnClient.read();
        rbnLineBuffer[bi++] = c;

        // If we are the end of line then break the loop
                    
        if (('\n' == c) || ('\r' == c))
          break;
      }

      //Serial.write(rbnLineBuffer);
          
      // Do we have something worth processing
          
      if (strlen(rbnLineBuffer) > 2) {
            
        // Is this the please enter request?

        if (NULL != strstr(rbnLineBuffer,"Please")) {

          // Send back the callsign
              
          rbnClient.println(callsign);
        }
        else {
              
          // We might have a real spot to process
              
          //Serial.write(rbnLineBuffer);

          // Split the string up

          char **parts;

          int n = SplitStringOnSpace(rbnLineBuffer, &parts);

          // Do we have the correct number of parts
              
          if ((n == 12) || (n == 13)) {

            // We have a valid RBN spot, pass it on for processing

            ledSetIndexColour(LED_DATA,CRGB::Green);

            char *spotter = parts[RBN_SPOTTER];
            char *spotted = parts[RBN_SPOTTED];
            double frequency = atof(parts[RBN_FREQUENCY]);
            char *rbnType = parts[RBN_TYPE];
            char *rbnTime = parts[12 == n ? RBN_TIME : RBN_TIME + 1];

            // Before we pass the data back we'll clean it up a little

            // Remove amy trailing -<text> from the spotter
            
            char *minusSign = strchr(spotter,'-');
            if (NULL != minusSign) {
              *minusSign = '\0';
            }

            // Remove amy trailing \r from the rbnTime
            
            char *carraigeReturn = strchr(rbnTime,'\r');
            if (NULL != carraigeReturn) {
              *carraigeReturn = '\0';
            }            

            // Pass the data on
            
            spotHandler(spotter,spotted,frequency,rbnType,rbnTime);
            
            beaconsSpotted(spotter,spotted,rbnType,frequency);

            ledSetIndexColour(LED_DATA,CRGB::Black);
          }
        }
      }
      else {

        // ?
        
      }

      // Put in a short delay, just to help keep other threads active

      delay(1);
    }

    //digitalWrite(LED_PIN,LOW);
  }
  else {

    // We seem to have lost connection to the Reverse Beacon Network. Try
    // to connect again!
        
    //currentState = STATE_CONNECTING_TO_REVERSE_BEACON_NETWORK;
  }

  return retVal;
}
