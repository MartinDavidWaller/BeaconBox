/*
 *  WebServer.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
extern void webServerSetUp(void configurationUpdateHandler(), void modeChangeHandler());
extern void sendToRBNDataListeners(char *spotter, char*spotted, double frequency, char *rbnTime);
extern void sendBeaconColourToBeaconListeners(char *beacon, char *colour, char *subText);
extern void sendFrequencyColourToBeaconListeners(double frequency, char *colour);
extern void sendAllBeaconsOffToBeaconListeners();
extern void sendBeaconOnOffToBeaconListeners(char *beacon, bool onOff);
extern void sendAllFrequenciesOffToBeaconListeners();
extern void sendFrequencyActiveInActiveToBeaconListeners(double frequency, bool activeInActive);
