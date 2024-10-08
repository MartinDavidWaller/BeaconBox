/*
 *  Beacons.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#define CS_4S7B "4S7B"
#define CS_4U1UN "4U1UN"
#define CS_4X6TU "4X6TU"
#define CS_5Z4B "5Z4B"
#define CS_CS3B "CS3B"
#define CS_JA2IGY "JA2IGY"
#define CS_KH6RS "KH6RS"
#define CS_LU4AA "LU4AA"
#define CS_OA4B "OA4B"
#define CS_OH2B "OH2B"
#define CS_RR90 "RR90"
#define CS_VE8AT "VE8AT"
#define CS_VK6RBP "VK6RBP"
#define CS_VR2B "VR2B"
#define CS_W6WX "W6WX"
#define CS_YV5B "YV5B"
#define CS_ZL6B "ZL6B"
#define CS_ZS6DN "ZS6DN"

#define B_4S7B 0
#define B_4U1UN 1
#define B_4X6TU 2
#define B_5Z4B 3
#define B_CS3B 4
#define B_JA2IGY 5
#define B_KH6RS 6
#define B_LU4AA 7
#define B_OA4B 8
#define B_OH2B 9
#define B_RR90 10
#define B_VE8AT 11
#define B_VK6RBP 12
#define B_VR2B 13
#define B_W6WX 14
#define B_YV5B 15
#define B_ZL6B 16
#define B_ZS6DN 17

#define NUMBER_OF_BEACONS 18

#define F_14100 0
#define F_18110 1
#define F_21150 2
#define F_24930 3
#define F_28200 4

#define NUMBER_OF_FREQUENCIES 5

 // Define routines and functions

void beaconsSetUp();
void beaconsSpotted(char *spotter, char* spotted, double frequency);
 
