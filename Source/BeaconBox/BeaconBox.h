/*
 *  BeaconBox.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// The following manifests provide the product information

#define PROGRAM_NAME "BeaconBox"
#define PROGRAM_VERSION_MAJOR 0
#define PROGRAM_VERSION_MINOR 7
#define COPYRIGHT_TEXT "(C) M.D.Waller G0PJO"

// ----

#define REVERSE_BEACON_NETWORK_ADDRESS "telnet.reversebeacon.net"
#define REVERSE_BEACON_NETWORK_PORT 7000

#define DEFAULT_SPOTTER_WILDCARDS "G*"
#define DEFAULT_TIMEZONE "GMT0BST,M3.5.0/01,M10.5.0/02"

#define DEFAULT_SPOTTER_TIMEOUT_MINUTES 5
#define DEFAULT_FREQUENCY_STEP_TIME_SECONDS 5
#define DEFAULT_LED_BRIGHTNESS 255

// The mode pin must support pullout. See https://forum.arduino.cc/t/esp32-pins-that-support-pullup/1173356/3

#define MODE_PIN 25

// LEDs - two LED types are supported WS2812B and WS2812D (8mm)

#define LED_TYPE_WS2812B 1
#define LED_TYPE_WS2812D 2

#define LED_TYPE LED_TYPE_WS2812B

// Supported Builds

#define BUILD_TYPE_TEST_BOARD 1
#define BUILD_TYPE_YAESU_MAP 2

#define BUILD_TYPE BUILD_TYPE_TEST_BOARD

// Define the animation defaults

#define DEFAULT_ANIMATION_ENABLED true
#define DEFAULT_ANIMATION_BEACONS_HEARD_DURATION_SECONDS 30
#define DEFAULT_ANIMATION_BEACONS_ACTIVE_DURATION_SECONDS 30
#define DEFAULT_ANIMATION_BEACONS_IN_DAYLIGHT_DURATION_SECONDS 10
#define DEFAULT_ANIMATION_MANUAL_MODE_TIMEOUT_SECONDS 60

// The following enumeration is used to control the operation mode

typedef enum {
  
  OPERATION_MODE_BEACONS_HEARD,     // Usual mode, reporting beacons that have been heard
  OPERATION_MODE_NCDXF_IARU,        // NCDXF/IARU Transmission schedule mode
  OPERATION_MODE_DAYLIGHT           // Daylight mode
  
} OPERATION_MODE;

// The following are used to control DEBUG output

#define DEBUG_BEACONS_IN_DAYLIGHT 1
#define DEBUG_SCAN_SSIDS 1

// Defined methods / data

extern OPERATION_MODE activeMode;
