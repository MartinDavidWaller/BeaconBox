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
  * 
  * Make use of DEFAULT_BEACON_TIMEOUT_MINUTES.
  * 
  * https://mapsvg.com/maps/world
  * 
  * Map from https://www.fla-shop.com/svg/
  */

  /*
   * EAGLE PCB Libraries
   * 
   * ESP32 - 30 Pin - https://github.com/shridattdudhat/ESP32-DEVKITV1
   * ESP32 - 38 Pin - https://hackaday.io/project/46280/files             DevKitC
   */

  /*
   * Read all the flash
   * 
   * C:\Users\marti>C:\Users\marti\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.6/esptool.exe --chip esp32 --port COM4 --baud 921600 read_flash 0 ALL flash_contents.bin
   * 
   * Erase all the flash
   * 
   * C:\Users\marti>C:\Users\marti\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.6/esptool.exe --chip esp32 --port COM4 --baud 921600 erase_flash
   * 
   * Write flash
   * 
   * C:\Users\marti>C:\Users\marti\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.6/esptool.exe --chip esp32 --port COM4 --baud 921600 write_flash 0x0 flash_contents.bin
   */
   
#include "Arduino.h"

#include <FastLED.h>
#include "SPIFFS.h"
#include <WiFi.h>

#include "Animation.h"
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
OPERATION_MODE activeMode;                        // Active mode
OPERATION_MODE requestedMode;                     // Requested mode, changed by the mode switch
RUNNING_STATE currentState;                       // Current running state
time_t startUpTime;                               // Startup time

boolean startAP() {

  #define START_AP_DELAY 1000
  
  // Start the Access Point
  
  boolean result = WiFi.softAP(PROGRAM_NAME);

  // Return the result
  
  return result;
}  

void stepTheMode() {

  switch(activeMode) {

    case OPERATION_MODE_BEACONS_HEARD:
      requestedMode = OPERATION_MODE_NCDXF_IARU;
      break;
        
    case OPERATION_MODE_NCDXF_IARU:
      requestedMode = OPERATION_MODE_DAYLIGHT;
      break;
        
    case OPERATION_MODE_DAYLIGHT:
      requestedMode = OPERATION_MODE_BEACONS_HEARD;
      break;

    default:
      break;
  }  
}

void IRAM_ATTR switchInterrupt() {

  static unsigned long last_interrupt_time = 0;

  // Get the interrupt time
  
  unsigned long interrupt_time = millis();
  
  // Is it 200 ms greater than the last interrupt, if not ignore it
  
  if (interrupt_time - last_interrupt_time > 200) 
  {
    // Update the requested mode depending on the active node. Basically
    // switch it between the available modes

    stepTheMode();

    // Tell the animation

    animationManualEvent();
  }

  // Update the last interrupt time
  
  last_interrupt_time = interrupt_time;
}

void configurationUpdateHandler() {

  // Pass on any change to the LED brightness

  ledBrightness(configuration.LEDBrightness);
}

void modeChangeHandler(bool manualEvent) {

  // If this is a manual event then we may need to step the mode

  if (true == manualEvent) {

    // Tell the animation about the manual event
    
    animationManualEvent();
  }
  
  // Step the moode

  stepTheMode();
}

// This is the setup routine. It all starts here.

void setup() {

  // Set up the serial port so we can generate debug etc.
  
  Serial.begin(115200);\
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  } 

  Serial.println("Up and running");
  
  // Set up the mode button

  pinMode(MODE_PIN,INPUT_PULLUP);

  Serial.printf("********** %d\n",digitalRead(MODE_PIN));

  // Set the active mode and requested mode to the default

  activeMode = OPERATION_MODE_BEACONS_HEARD;
  requestedMode = OPERATION_MODE_BEACONS_HEARD;

  // Setup the LEDs and put on a short display.

  ledChainSetUp();
  
  // Read the configuration data from the EEPROM

  Serial.println("");
  Serial.println("Read the Configuration Data");
  bool checkSumOK = readConfiguration(&configuration);

  //Serial.println("");
  //hexDump((char *)&configuration,sizeof(Configuration));

  // If the checksum is not OK or if the version numbers
  // do not match then we must start again!

  if ((false == checkSumOK) || 
      (configuration.MajorVersion != PROGRAM_VERSION_MAJOR) || 
      (configuration.MinorVersion != PROGRAM_VERSION_MINOR) ||
      (0 == digitalRead(MODE_PIN))) {

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
    strcpy((char*)&configuration.Hostname[0],PROGRAM_NAME);
    strcpy((char*)&configuration.TimeZone[0],DEFAULT_TIMEZONE);
    strcpy((char*)&configuration.SpotterWildcards[0],DEFAULT_SPOTTER_WILDCARDS);
    configuration.SpotterTimeOutMinutes = DEFAULT_SPOTTER_TIMEOUT_MINUTES;
    configuration.FrequencyStepTimeSeconds = DEFAULT_FREQUENCY_STEP_TIME_SECONDS;
    configuration.LEDBrightness = DEFAULT_LED_BRIGHTNESS;

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

  // Blink the LEDs

  ledBrightness(configuration.LEDBrightness);
  ledChainBlinkAll();
    
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

  // Set the start time

  time(&startUpTime);

  // Setup the progress LEDs

  // ***************** progressSetUp();
  
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
    
    // ***************** progressAccessPointOpen();
  }

  // Setup the interrupt service routine

  attachInterrupt(MODE_PIN, switchInterrupt, FALLING);
  
  // Setup the beacons

  beaconsSetUp(&configuration.SpotterWildcards[0]);

  // Next setup the Web Server

  webServerSetUp(&configurationUpdateHandler, &modeChangeHandler);

  // Next setup the animation

  animationSetUp(&modeChangeHandler);

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

void spotHandler(char *spotter, char*spotted, double frequency, char *rbnTime) {

  //Serial.printf("spotHandler: spotter: %s, spotted %s, frequency %f, time %s\n", spotter, spotted, frequency, rbnTime); 

  sendToRBNDataListeners(spotter, spotted, frequency, rbnTime);
}

time_t lastBeaconsHeard = -1;

void doBeaconsHeardMode() {

  // Here we are displaying beacons that have been heard.
  
  // We need to decide if the time is right to step on to the
  // next beacon frequency

  // Get the time
  
  time_t timeNow;
  time(&timeNow);

  // Is it time to update the beacons frequency?
  
  if ((-1 == lastBeaconsHeard) || (timeNow > lastBeaconsHeard + configuration.FrequencyStepTimeSeconds)) {

    // Yes, step the beacons
    
    dumpBeacons();
    beaconsStepBeacon();

    // Update the time of the last step
    
    time(&lastBeaconsHeard);
  }
}

time_t lastBeaconsActive = -1;

void doBeaconsActiveMode() {

  // Here we are displaying beacons that are active.
  
  // We need to decide if the time is right to move on

  // Get the time
  
  time_t timeNow;
  time(&timeNow);

  // Is it time to update the beacons frequency?
  
  if ((-1 == lastBeaconsActive) || (timeNow > lastBeaconsActive + 1)) {

    // Yes, update the active beacons
    
    beaconsShowActiveBeacons();

    // Update the time of the last step
    
    time(&lastBeaconsActive);
  }
}

time_t lastDaylight = -1;

void doDaylightMode() {

  // Here we are displaying beacons that are in daylight.
  
  // We need to decide if the time is right to move on

  // Get the time
  
  time_t timeNow;
  time(&timeNow);

  // Is it time to update the beacons in daylight
  
  if ((-1 == lastBeaconsActive) || (timeNow > lastDaylight + 60)) {

    // Yes, update the beacons in daylight
    
    beaconsShowBeaconsInDaylight();

    // Update the time of the last step
    
    time(&lastDaylight);
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

        bool connected = rbnClientConnect(REVERSE_BEACON_NETWORK_ADDRESS, REVERSE_BEACON_NETWORK_PORT, spotHandler);

        if (true == connected) {

          Serial.printf("\nConnected to RBN %s:%d\n",REVERSE_BEACON_NETWORK_ADDRESS, REVERSE_BEACON_NETWORK_PORT); 

          // Update the current state to receiving RBN data
        
          currentState = STATE_RECIEVING_RBN_DATA;     

          // Update the animation

          animationSetState(true, 
            ANIMATION_MODE_HEARD_TIMEOUT_SECONDS, 
            ANIMATION_MODE_NCDXFIARU_TIMEOUT_SECONDS, 
            ANIMATION_MODE_DAYLIGHT_TIMEOUT_SECONDS,
            ANIMATION_MANUAL_EVENT_TIMEOUT_SECONDS);
        }
        else {

          // Not connected

           Serial.printf("\nFailed to connect to RBN %s:%d\n",REVERSE_BEACON_NETWORK_ADDRESS, REVERSE_BEACON_NETWORK_PORT); 
        }
      }
        
      break;    

    case STATE_RECIEVING_RBN_DATA:

      // Have we changed mode?

      if (activeMode == requestedMode) {

        // No change, decode the active mode and action it

        switch(activeMode) {
          case OPERATION_MODE_BEACONS_HEARD:

            doBeaconsHeardMode();
            break;
            
          case OPERATION_MODE_NCDXF_IARU:

            doBeaconsActiveMode();
            break;
            
          case OPERATION_MODE_DAYLIGHT:

            doDaylightMode();
            break;
        }

        // Process any animation

        animationAnimate();
      }
      else {

        // Here we have a change of mode!

        // Clear all frequency LEDs before moving on

        ledTurnOffAllFrequencyLeds();

        // What's changing

        switch(requestedMode) {
          
          case OPERATION_MODE_BEACONS_HEARD:

            // Usual mode, reporting beacons that have been heard. All we
            // need to do is reset the timer.

            lastBeaconsHeard = -1;
            break;
            
          case OPERATION_MODE_NCDXF_IARU:

            // NCDXF/IARU Transmission schedule mode

            // Clear down all beacons LED

            ledTurnOffAllBeaconLeds();
          
            // Reset the time

            lastBeaconsActive = -1;
          
            // Set the frequency colours
          
            beaconsShowFrequencyColours();

            // Clean up all beacons

            clearActiveBeaconColours();
            sendAllBeaconsOffToBeaconListeners();         
            break;
            
          case OPERATION_MODE_DAYLIGHT:

            // Reset the time

            lastDaylight = -1;
            
            // Turn off all the frequency leds
            
            ledTurnOffAllFrequencyLeds();

            // Ditto on any webpage
            
            sendAllFrequenciesOffToBeaconListeners();
            break;          
        }
        
        // Finally make this the active mode

        activeMode = requestedMode;
      }
      
      // Whatever mode we are in we need to process any data that we have received
      // from the reverse beacon network.
      
      bool stillConnected = rbnClientProcessData((char*)&configuration.Callsign[0]);

      // Are we still connected?

      if (false == stillConnected) {

        // No, update the state

        currentState = STATE_CONNECTING_TO_REVERSE_BEACON_NETWORK;
      }

      break;        
  }

  // This delay is important. It stops other threads being starved
  // of CPU time.
  
  delay(1) ;
}
