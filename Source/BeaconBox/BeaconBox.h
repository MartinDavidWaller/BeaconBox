/*
 *  BeaconBox.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// The following manifests provide the product information

#define PROGRAM_NAME "BeaconBox"
#define PROGRAM_VERSION_MAJOR 0
#define PROGRAM_VERSION_MINOR 6
//define BETA_TEXT ""
#define COPYRIGHT_TEXT "(C) M.D.Waller G0PJO"

// ----

#define REVERSE_BEACON_NETWORK_ADDRESS "telnet.reversebeacon.net"
#define REVERSE_BEACON_NETWORK_PORT 7000

#define DEFAULT_SPOTTER_WILDCARDS "G*"
#define DEFAULT_TIMEZONE "GMT0BST,M3.5.0/01,M10.5.0/02"

#define DEFAULT_SPOTTER_TIMEOUT_MINUTES 5
#define DEFAULT_FREQUENCY_STEP_TIME_SECONDS 5
#define DEFAULT_LED_BRIGHTNESS 255

//define DEBUG 0

//define MAX_SECONDS_BEFORE_RESTART 60

// The mode pin must support pullout. See https://forum.arduino.cc/t/esp32-pins-that-support-pullup/1173356/3

#define MODE_PIN 25

// LEDs - two LED types are supported WS2812B and WS2812D (8mm)

#define LED_TYPE_WS2812B 1
#define LED_TYPE_WS2812D 2

#define LED_TYPE LED_TYPE_WS2812B

// Define the animation defaults

#define ANIMATION_ACTIVE false;
#define ANIMATION_MODE_HEARD_TIMEOUT_SECONDS 30
#define ANIMATION_MODE_NCDXFIARU_TIMEOUT_SECONDS 30
#define ANIMATION_MODE_DAYLIGHT_TIMEOUT_SECONDS 10
#define ANIMATION_MANUAL_EVENT_TIMEOUT_SECONDS 60

// The following enumeration is used to control the operation mode

typedef enum {
  
  OPERATION_MODE_BEACONS_HEARD,     // Usual mode, reporting beacons that have been heard
  OPERATION_MODE_NCDXF_IARU,        // NCDXF/IARU Transmission schedule mode
  OPERATION_MODE_DAYLIGHT           // Daylight mode
  
} OPERATION_MODE;

// Defined methods / data

extern OPERATION_MODE activeMode;
//extern struct RuntimeData runtimeData;
//extern bool doSaveDirtyFriends;
//extern bool doListenWithVolume;
//extern int doListenWithVolumeVolume;
//extern void playSound(const int soundToPlay);
//extern void setNewVolume();
