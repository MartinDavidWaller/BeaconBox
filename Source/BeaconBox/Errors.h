/*
 *  Errors.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

 #define ERROR_STATE_0_LED LED_BEACON_FREQUENCY_14100
 #define ERROR_STATE_1_LED LED_BEACON_FREQUENCY_18110
 #define ERROR_STATE_2_LED LED_BEACON_FREQUENCY_21150
 #define ERROR_STATE_3_LED LED_BEACON_FREQUENCY_24930
 #define ERROR_STATE_4_LED LED_BEACON_FREQUENCY_28200

 // Define routines and functions

void errorsSetUp();
void errorOpeningSpiffs();
void errorOpeningAccessPoint();
void errorSettingsRequired();
void errorConnectingToWiFi();
