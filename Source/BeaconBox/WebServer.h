/*
 *  WebServer.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
extern void webServerSetUp();
extern void sendToRBNDataListeners(char *spotter, char*spotted, double frequency, char *rbnTime);
extern void sendBeaconColourToBeaconListeners(char *beacon, char *colour);
extern void sendFrequencyColourToBeaconListeners(double frequency, char *colour);
