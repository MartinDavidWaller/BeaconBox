/*
 *  BeaconBox.ino
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

 /*
  * Issues:
  * 
  * When returning SSIDs we need to provide a proper name without spaces etc so the load of the options works? Or
  * at least that's what I think is making it fail.
  */
  
 #include "Arduino.h"

#include <FastLED.h>
#include "SPIFFS.h"
#include <WiFi.h>

#include "BeaconBox.h"
#include "Beacons.h"
#include "Configuration.h"
#include "Dump.h"
#include "LEDChain.h"
#include "Progress.h"
#include "RBNClient.h"
#include "WebServer.h"

// The following manifests are used to control the active state

typedef enum {
  
  //STATE_BUILDING_ACCESS_POINT,
  STATE_CONNECTING_TO_WIFI,
  STATE_CONNECTING_TO_REVERSE_BEACON_NETWORK,
  STATE_RECIEVING_RBN_DATA
  
} RUNNING_STATE;

// Define the working data

struct Configuration configuration;               // Configuration object
RUNNING_STATE currentState;                       // Current running state

boolean startAP() {

  #define START_AP_DELAY 1000
  
  // Start the Access Point
  
  boolean result = WiFi.softAP(PROGRAM_NAME);

  // Return the result
  
  return result;
}  

// This is the setup routine. It all starts here.

void setup() {

  // Set up the serial port so we can generate debug etc.
  
  Serial.begin(115200);\
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  } 
  
  Serial.println("Up and running");

  // Setup the LEDs and put on a short display.

  ledChainSetUp();
  ledChainBlinkAll();

  // Read the configuration data from the EEPROM

  Serial.println("");
  Serial.println("Read the Configuration Data");
  bool checkSumOK = readConfiguration(&configuration);

  Serial.println("");
  hexDump((char *)&configuration,sizeof(Configuration));

  // If the checksum is not OK or if the version numbers
  // do not match then we must start again!

  if ((false == checkSumOK) || 
      (configuration.MajorVersion != PROGRAM_VERSION_MAJOR) || 
      (configuration.MinorVersion != PROGRAM_VERSION_MINOR) ||
      (0 == digitalRead(BOOT_PIN))) {

    // Here the versions don't match or the checksum is wrong or
    // the boot pin is pressed.

    if (false == checkSumOK) {
      Serial.println("");
      Serial.println("Invalid checksum.");       
    }
    else if ((configuration.MajorVersion != PROGRAM_VERSION_MAJOR) || 
             (configuration.MinorVersion != PROGRAM_VERSION_MINOR)) {

      Serial.println("");
      Serial.println("Different verison numbers.");
      Serial.print("...Found V");
      Serial.print(configuration.MajorVersion);
      Serial.print(".");
      Serial.println(configuration.MinorVersion);
      Serial.print("...Expected V");
      Serial.print(PROGRAM_VERSION_MAJOR);
      Serial.print(".");
      Serial.println(PROGRAM_VERSION_MINOR);    
      Serial.println("");              
    }
    else {
      Serial.println("");
      Serial.println("Reset switch active.");      
    }
      
    // Initialise the configuration data

    memset(&configuration,0,sizeof(Configuration));
    configuration.MajorVersion = PROGRAM_VERSION_MAJOR;
    configuration.MinorVersion = PROGRAM_VERSION_MINOR;
    configuration.Flags = CLEAR;
    //SET_LED_ENABLED((&configuration));
    //SET_SOUND_ENABLED((&configuration));
    strcpy((char*)&configuration.Hostname[0],PROGRAM_NAME);
    strcpy((char*)&configuration.TimeZone[0],DEFAULT_TIMEZONE);
    strcpy((char*)&configuration.SpotterWildcards[0],DEFAULT_SPOTTER_WILDCARDS);
    //configuration.Volume = DEFAULT_VOLUME;
    //configuration.MinutesBetweenAlerts = DEFAULT_MINUTES_BETWEEN_ALERTS;
    //configuration.FriendCycleCount = DEFAULT_FRIEND_CYCLE_COUNT;

    Serial.println("Writing new configuration");
    Serial.println("");
    hexDump((char *)&configuration,sizeof(Configuration));

    // Write it out
    
    writeConfiguration(&configuration); 

    // Next we need to setup the access point.
  }

  // Dump the current configuration
  
  dumpConfiguration(&configuration);    

  // We are going to need access to the files stored so start up SPIFFS
  // and list out what we have.

  Serial.println("");
  Serial.println("Contents of SPIFFS:");
  Serial.println("");

  if(false == SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  // Open the root object
  
  File root = SPIFFS.open("/");

  // Loop through the files
  
  File file = root.openNextFile();
  while (file) {
    if (true == file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());    
    } else {
      Serial.printf("  FILE: %s %d\n", file.name(), file.size());
    }

    // Move to the next file
    
    file = root.openNextFile();
  }  

  // Set the time server

  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", (char*)&configuration.TimeZone[0],1);

  // Setup the progress LEDs

  progressSetUp();
  
  // Next start the Access Point

  bool accessPointStarted = startAP();
  if (false == accessPointStarted) {
    Serial.println("");
    Serial.println("Failed to start the access point!");
  }
  else
  {
    // Display the local IP address
    
    IPAddress ipAddress = WiFi.softAPIP();
    Serial.printf("\nLocal Access Point IP: http://%s\n",ipAddress.toString().c_str());
        
    // Update the progress LEDs
    
    progressAccessPointOpen();
  }

  // Setup the beacons

  beaconsSetUp(&configuration.SpotterWildcards[0]);

  // Next setup the Web Server

  webServerSetUp();

  // Set the current state

  currentState = STATE_CONNECTING_TO_WIFI;  
}

void connectToWiFi() {
  
  // We are about to connect to the WiFi. We need to ensure that we
  // have the necessary data to try.

  if (0 == strlen((char *)&configuration.WiFi_SSID[0])) {

    // We don't currently have a SSID, we need to comminicate this to the
    // end user somehow!
  }
  else {

    // We have a SSID, etc try to make the connection

    /*
     * If you do disconnect here then the user is thrown off the access point.
     * 
     * Disconnect anything that we may have
     *
     * WiFi.disconnect();
     */

    // Apply any hostname that we may have. If we done have one then we
    // can default it to the program name

    Serial.printf("\nSetting hostname: %s\n",(char *)&configuration.Hostname[0]);
    if (strlen((char *)&configuration.Hostname[0]) > 0)
      WiFi.setHostname((char *)&configuration.Hostname[0]);
    else
      WiFi.setHostname(PROGRAM_NAME);    
        
    // Begin the connection
  
    WiFi.begin((char *)&configuration.WiFi_SSID[0],(char *)&configuration.WiFi_Password[0]);

    // Disable sleep
  
    WiFi.setSleep(false);
    
    // Wait for the connection to be made.

    int maxTry = 10;
    while ((WiFi.status() != WL_CONNECTED) && (maxTry > 0)) {

      // Wait and update the try count.

      delay(1000);
      maxTry--;
    }

    // Did we manage to connect?

    if (WiFi.status() != WL_CONNECTED) {

      // Here the connection has failed

       Serial.printf("\nConnected to WiFi failed!\n");
    }
    else {

      // Here we are connected to the WiFi

      IPAddress ipAddress = WiFi.localIP();

      Serial.printf("\nConnected to WiFi: http://%s\n",ipAddress.toString().c_str()); 
      
      // Kick off a WiFi Scan, we may need the SSID list later.

      WiFi.scanNetworks(true);

      // Move to the next state
          
      currentState = STATE_CONNECTING_TO_REVERSE_BEACON_NETWORK;
    }
  }
}

void loop() {
  
  // What we do here depend on what state we are in.

  switch(currentState) {

    case STATE_CONNECTING_TO_WIFI:

      // Make the connection to the WiFi
      
      connectToWiFi();
      break;

    case STATE_CONNECTING_TO_REVERSE_BEACON_NETWORK:

      // Make sure that we are connected to the WiFi

      if (WiFi.status() != WL_CONNECTED)
      {
        // Revert to connecting to the WiFi
        
        currentState = STATE_CONNECTING_TO_WIFI;
      }
      else
      {
        // Try connecting to the Reverse Beacon Network

        bool connected = rbnClientConnect(REVERSE_BEACON_NETWORK_ADDRESS, REVERSE_BEACON_NETWORK_PORT);

        if (true == connected) {

          Serial.printf("\nConnected to RBN %s:%d\n",REVERSE_BEACON_NETWORK_ADDRESS, REVERSE_BEACON_NETWORK_PORT); 

          // Update the current state to receiving RBN data
        
          currentState = STATE_RECIEVING_RBN_DATA;          
        }
        else {

          // Not connected

           Serial.printf("\nFailed to connect to RBN %s:%d\n",REVERSE_BEACON_NETWORK_ADDRESS, REVERSE_BEACON_NETWORK_PORT); 
        }
      }
        
      break;    

    case STATE_RECIEVING_RBN_DATA:

      // Process any data that we have
      
      bool stillConnected = rbnClientProcessData((char*)&configuration.Callsign[0]);

      // Are we still connected?

      if (false == stillConnected) {

        // No, update the state

        currentState = STATE_CONNECTING_TO_REVERSE_BEACON_NETWORK;
      }
            
      break;        
  }
}
