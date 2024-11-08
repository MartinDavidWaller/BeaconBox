/*
 *  Configuration.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#define EEPROM_SIZE 512

#define HOSTNAME_SIZE 15
#define WIFI_SSID_SIZE 32
#define WIFI_PASSWORD_SIZE 64
#define CALLSIGN_SIZE 20
#define SPOTTER_WILDCARD_SIZE (CALLSIGN_SIZE * 10)
#define TIMEZONE_SIZE 30

// The configuration structure holds the basic configuration data.

struct Configuration
{
  uint8_t MajorVersion;
  uint8_t MinorVersion;
  char Hostname[HOSTNAME_SIZE + 1];
  char WiFi_SSID[WIFI_SSID_SIZE + 1];
  char WiFi_Password[WIFI_PASSWORD_SIZE + 1];
  char Callsign[CALLSIGN_SIZE + 1];
  char SpotterWildcards[SPOTTER_WILDCARD_SIZE + 1];
  char TimeZone[TIMEZONE_SIZE + 1];
  uint8_t SpotterTimeOutMinutes;
  uint8_t FrequencyStepTimeSeconds;
  uint8_t LEDBrightness;

  // Animation Specific Settings

  bool AnimationEnabled;
  uint8_t BeaconsHeardDurationSeconds;
  uint8_t BeaconsActiveDurationSeconds;
  uint8_t BeaconsInDaylightDurationSeconds;
  uint8_t ManualModeTimeoutSeconds;
};

// The EPPROM_DATA structure wraps the configuration structure and adds a checksum
// to it. The checkum will be validated on reads and updated on writes.

struct EEPROM_DATA
{
  struct Configuration Configuration;
  uint16_t Size;
  uint16_t CheckSum;
};

// The following methods are used to manipulate the configuration information

bool readConfiguration(struct Configuration *configuration);
void writeConfiguration(struct Configuration *configuration);
void dumpConfiguration(struct Configuration *configuration);
