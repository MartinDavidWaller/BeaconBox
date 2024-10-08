/*
 *  RBNClient.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include "Arduino.h"
#include <WiFi.h>
#include "Beacons.h"
#include "RBNClient.h"
#include "StringHelper.h"

WiFiClient rbnClient;                             // Telnet client
char rbnLineBuffer[RBN_LINE_BUFFER_SIZE + 1];     // Line buffer for the RBN data

bool rbnClientConnect(char *address, int port) {
  
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
      memset(rbnLineBuffer,0,2048 + 1);
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

            beaconsSpotted(
              parts[RBN_SPOTTER],
              parts[RBN_SPOTTED],
              atof(parts[RBN_FREQUENCY]));
            
            // Update the callsign count
                
            //runtimeData.SpotsSeen++;
                
            // Update the last spot seen date/time

            //time(&runtimeData.LastSpotSeen);
                
            // Process the callsign
                
            //struct Friend *_friend = ProcessCall(
              //parts[RBN_CALLSIGN],
              //atoi(parts[RBN_SPEED]),
              //parts[RBN_SPEED_UNITS],
              //atof(parts[RBN_FREQUENCY]) / 1000.0f,
              //parts[RBN_MODE],
              //rbnLineBuffer
            //);
                
            // Did we find a friend?
                
            //if (NULL != _friend) {

              // Yes, we have just seen a friend. We need to locate the FriendExt record
              // and decide if we need to make a sound!

              //struct FriendExt *friendExt = GetFriendExt(_friend);

              // We are going to need to the time to decide if we sound an alert
              // and to update the FriendExt record

              //time_t now;
              //time(&now);

              // Do we action this alert

              //if (now - friendExt->LastAlert > configuration.MinutesBetweenAlerts * 60) {

                // Yes we action the alert.

                //currentRBNState = DISPLAY_FRIEND_ALERT;
                //displayFriendAlert(_friend);  
  
                // Do we want to make a noise?

                //if ((true == IS_SOUND_ENABLED((&configuration))) &&
                //    (true == IS_FRIEND_SOUND_ENABLED(_friend))) {

                  // Yes we do, we need to sound an alert.

                  //PlaySound(_friend->Sound);                    
                //}

                // Update the last alert time

                //friendExt->LastAlert = now;                    
              //}                    
            //}
          }
        }
      }
      else {

        // ?
        
      }
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
