/*
 *  RBNClient.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#define RBN_LINE_BUFFER_SIZE 2048

// The following manifests define the data order in the line recieved from
// the Reverse Beacon Network. A typical RBN spot line looks like:
//
// DX de K3LR-#:    14042.0  GB2RN          CW    12 dB  18 WPM  CQ      1350Z

#define RBN_SPOTTER 2
#define RBN_FREQUENCY 3
#define RBN_SPOTTED 4
#define RBN_MODE 5
#define RBN_SPEED 8
#define RBN_SPEED_UNITS 9
#define RBN_TYPE 10
#define RBN_TIME 11

// Define routines and functions

bool rbnClientConnect(char *address, int port, void spotHandler(char *spotter, char*spotted, double frequency, char *rbnType, char *rbnTime));
bool rbnClientProcessData(char *callsign);
