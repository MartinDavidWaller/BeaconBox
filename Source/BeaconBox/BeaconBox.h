/*
 *  BeaconBox.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

// The following manifests provide the product information

#define PROGRAM_NAME "BeaconBox"
#define PROGRAM_VERSION_MAJOR 0
#define PROGRAM_VERSION_MINOR 1
#define BETA_TEXT ""
#define COPYRIGHT_TEXT "(C) M.D.Waller G0PJO"

// CHANGE THESE LINES TO TURN OFF THE DISPLAY BACK LIGHT BETWEEN TWO TIMES ----

//define TURN_BACK_LIGHT_OFF_YES 1
//define TURN_BACK_LIGHT_OFF_NO 0

//define TURN_BACK_LIGHT_OFF TURN_BACK_LIGHT_OFF_NO

//define BACK_LIGHT_ON_HOUR 06
//define BACK_LIGHT_ON_MINUTE 00

//define BACK_LIGHT_OFF_HOUR 21
//define BACK_LIGHT_OFF_MINUTE 00

// ----

#define REVERSE_BEACON_NETWORK_ADDRESS "telnet.reversebeacon.net"
#define REVERSE_BEACON_NETWORK_PORT 7000

#define DEFAULT_SPOTTER_WILDCARDS "G*"
#define DEFAULT_TIMEZONE "GMT0BST,M3.5.0/01,M10.5.0/02"


#define DEBUG 0

#define MAX_SECONDS_BEFORE_RESTART 60


// The mode pin must support pullout. See https://forum.arduino.cc/t/esp32-pins-that-support-pullup/1173356/3

#define MODE_PIN 25

//struct RuntimeData
//{
  //time_t LastSpotSeen;
  //long SpotsSeen;
  //time_t BootTime;
  //long RBNConnectionsMade;
//};

// Defined methods / data

extern struct RuntimeData runtimeData;
//extern bool doSaveDirtyFriends;
//extern bool doListenWithVolume;
//extern int doListenWithVolumeVolume;
//extern void playSound(const int soundToPlay);
//extern void setNewVolume();
