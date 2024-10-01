/*
 *  BeaconBox.ino
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
 #include "Arduino.h"

#include <FastLED.h>
#include <WiFi.h>

#include "BeaconBox.h"
#include "Configuration.h"
#include "Dump.h"
#include "LEDChain.h"

// Define the working data

struct Configuration configuration;               // Configuration object

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

  // Next start the Access Point

  bool accessPointStarted = startAP();
  if (false == accessPointStarted) {
    Serial.println("");
    Serial.println("Failed to start the access point!");
  }
  
}

void loop() {
  
  // put your main code here, to run repeatedly:

  delay (5000);
  ledChainBlinkAll();

}
