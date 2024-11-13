/*
 *  WebServer.cpp
 *
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include "Arduino.h"
#include "Esp.h"
#include "ArduinoJson.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "FormatHelper.h"
#include "SPIFFS.h"
#include <HardwareSerial.h>
#include <time.h>
#include <sys/time.h>
#include "Configuration.h"
#include "BeaconBox.h"

AsyncWebServer server(80);
AsyncWebSocket rbnDataWebSocket("/rbnData");
AsyncWebSocket beaconDataWebSocket("/beaconData");
StaticJsonDocument<200> beaconJSONOut;
StaticJsonDocument<200> rbnJSONOut;

void (*configurationUpdateHandler)();
void (*modeChangeHandler)(bool manualEvent);

extern time_t startUpTime;
extern struct Configuration configuration;

void onDoReboot(AsyncWebServerRequest *request){

  // Redirect to the index page

  request->redirect("index.html");

  ESP.restart();
}

void onGetModeChange(AsyncWebServerRequest *request){

  // Pass on the request

  modeChangeHandler(true);

  // Redirect to the index page,

  request->redirect("index.html");
}

void onDoSettingsUpdate(AsyncWebServerRequest *request){

  AsyncWebParameter* hostnameInputParam = request->getParam("hostnameInput");
  AsyncWebParameter* passwordInputParam = request->getParam("passwordInput");
  AsyncWebParameter* ssidInputParam = request->getParam("ssidInput");
  AsyncWebParameter* callsignInputParam = request->getParam("callsignInput");
  AsyncWebParameter* spotterWildcardsInputParam = request->getParam("spotterWildcardsInput");
  AsyncWebParameter* spotterTimeOutMinutesInputParam = request->getParam("spotterTimeOutMinutesInput");
  AsyncWebParameter* frequencyStepTimeSecondsInputParam = request->getParam("frequencyStepTimeSecondsInput");
  AsyncWebParameter* ledBrightnessInputParam = request->getParam("ledBrightnessInput");

  AsyncWebParameter* enableAnimationInputParam = request->getParam("enableAnimationInput");
  AsyncWebParameter* beaconsHeardDurationInputParam = request->getParam("beaconsHeardDurationInput");
  AsyncWebParameter* beaconsActiveDurationInputParam = request->getParam("beaconsActiveDurationInput");
  AsyncWebParameter* beaconsInDaylightDurationInputParam = request->getParam("beaconsInDaylightDurationInput");
  AsyncWebParameter* manualModeTimeoutInputParam = request->getParam("manualModeTimeoutInput");
            
  Serial.println("Update settings:");

  Serial.print("...hostnameInput = ");
  Serial.println(hostnameInputParam->value().c_str());

  Serial.print("...passwordInputParam = ");
  Serial.println(passwordInputParam->value().c_str());

  Serial.print("...ssidInputParam = ");
  Serial.println(ssidInputParam->value().c_str());

  Serial.print("...callsignInputParam = ");
  Serial.println(callsignInputParam->value().c_str());

  Serial.print("...spotterWildcardsInputnInputParam = ");
  Serial.println(spotterWildcardsInputParam->value().c_str());

  Serial.print("...spotterTimeOutInputParam = ");
  Serial.println(spotterTimeOutMinutesInputParam->value().c_str());

  Serial.print("...frequencyStepTimeSecondsInputParam = ");
  Serial.println(frequencyStepTimeSecondsInputParam->value().c_str());

  Serial.print("...ledBrightnessInputParam = ");
  Serial.println(ledBrightnessInputParam->value().c_str());

  strcpy((char*)&configuration.Hostname[0],hostnameInputParam->value().c_str());
  strcpy((char*)&configuration.WiFi_SSID[0],ssidInputParam->value().c_str());
  strcpy((char*)&configuration.WiFi_Password[0],passwordInputParam->value().c_str());
  strcpy((char*)&configuration.Callsign[0],callsignInputParam->value().c_str());
  strcpy((char*)&configuration.SpotterWildcards[0],spotterWildcardsInputParam->value().c_str());
  configuration.SpotterTimeOutMinutes = atoi(spotterTimeOutMinutesInputParam->value().c_str());
  configuration.FrequencyStepTimeSeconds = atoi(frequencyStepTimeSecondsInputParam->value().c_str());
  configuration.LEDBrightness = atoi(ledBrightnessInputParam->value().c_str());

  configuration.AnimationEnabled = (0 == strcmp("on", enableAnimationInputParam->value().c_str()));
  configuration.BeaconsHeardDurationSeconds = atoi(beaconsHeardDurationInputParam->value().c_str());
  configuration.BeaconsActiveDurationSeconds = atoi(beaconsActiveDurationInputParam->value().c_str());
  configuration.BeaconsInDaylightDurationSeconds = atoi(beaconsInDaylightDurationInputParam->value().c_str());
  configuration.ManualModeTimeoutSeconds = atoi(manualModeTimeoutInputParam->value().c_str());
  
  // Write it out

  writeConfiguration(&configuration);

  // Notify the world

  configurationUpdateHandler();

  // Redirect to the settings page

  request->redirect("settings.html");
}

void onGetSettingsData(AsyncWebServerRequest *request){

  AsyncResponseStream *response = request->beginResponseStream("text/xml");

  response->printf("<?xml version=\"1.0\" encoding=\"utf-16\"?>");
  response->printf("<SettingsData ");

  response->printf("Hostname=\"%s\" ",&configuration.Hostname[0]);
  response->printf("WiFiSSID=\"%s\" ",&configuration.WiFi_SSID[0]);
  response->printf("WiFiPassword=\"%s\" ",&configuration.WiFi_Password[0]);
  response->printf("Callsign=\"%s\" ",&configuration.Callsign[0]);
  response->printf("SpotterWildcards=\"%s\" ",&configuration.SpotterWildcards[0]);
  response->printf("SpotterTimeOutMinutes=\"%d\" ",configuration.SpotterTimeOutMinutes);
  response->printf("FrequencyStepTimeSeconds=\"%d\" ",configuration.FrequencyStepTimeSeconds);
  response->printf("LEDBrightness=\"%d\" ",configuration.LEDBrightness);

  response->printf("AnimationEnabled=\"%s\" ",configuration.AnimationEnabled ? "true" : "false");
  response->printf("BeaconsHeardDurationSeconds=\"%d\" ",configuration.BeaconsHeardDurationSeconds);
  response->printf("BeaconsActiveDurationSeconds=\"%d\" ",configuration.BeaconsActiveDurationSeconds);
  response->printf("BeaconsInDaylightDurationSeconds=\"%d\" ",configuration.BeaconsInDaylightDurationSeconds);
  response->printf("ManualModeTimeoutSeconds=\"%d\" ",configuration.ManualModeTimeoutSeconds);

  response->printf("/>");

  request->send(response);
}

AsyncResponseStream *response;

void onGetNameVersion(AsyncWebServerRequest *request){

  AsyncResponseStream *response = request->beginResponseStream("text/xml");

  response->printf("<?xml version=\"1.0\" encoding=\"utf-16\"?>");
  response->printf("<IndexNameVersion ");

  response->printf("Name=\"%s\" ",PROGRAM_NAME);
  response->printf("Version=\"V%d.%d\" ",PROGRAM_VERSION_MAJOR,PROGRAM_VERSION_MINOR);
  response->printf("Copyright=\"M.D.Waller G0PJO. (c) Copyright [YEAR]. All rights reserved.\"");

  response->printf("/>");

  request->send(response);
}

void onGetUpTime(AsyncWebServerRequest *request){

  //char lineBuffer[20 + 1];

  AsyncResponseStream *response = request->beginResponseStream("text/xml");

  response->printf("<?xml version=\"1.0\" encoding=\"utf-16\"?>");
  response->printf("<UpTime ");

  response->printf("UpTime=\"%s\" ",FormatUptime(&startUpTime));
  response->printf("FreeHeap=\"%d\"",ESP.getFreeHeap());

  response->printf("/>");

  request->send(response);
}

//
// This method is called to return all the available SSIDs
//
void onGetSIDDs(AsyncWebServerRequest *request){

  // Build a buffer to write into and zero terminate it
  
  char b[1024];
  b[0] = '\0';

#if DEBUG_SCAN_SSIDS > 0
  Serial.println("");
  Serial.println("Scanning for SSIDs");
#endif

  // We need to return a blob of XML containing the visible SSIDs

  strcpy(b,"<SSIDs>");

  int n = WiFi.scanComplete();
  if(n == -2) {
    WiFi.scanNetworks(true);
  } 
  else if(n) {

    // We have some SSIDs to return
    
    for (int i = 0; i < n; ++i) {
      
      // Add the SSID to the result. This doesn't always get the correct 
      // answers. If you have WiFi on two frequencies then any trailing 
      // index value may be missing.

      strcat(b,"<SSID Name = \"");
      strcat(b,WiFi.SSID(i).c_str());
      strcat(b,"\" />");

#if DEBUG_SCAN_SSIDS > 0
      Serial.println("... " + WiFi.SSID(i));
#endif

    //String ssid;
    //uint8_t encryptionType;
    //int32_t RSSI;
    //uint8_t BSSID;
    //int32_t channel;
    //if (true == WiFi.getNetworkInfo(i, &ssid, &encryptionType, &RSSI, &BSSID, &channel)) {

    //Serial.println(">>> " + ssid);
    }
    
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }

  // Complete the XML

  strcat(b,"</SSIDs>");

  // Send it to the server

  request->send(200, "text/xml", b);
}

void onBeaconDataWebSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

  Serial.println("onRBNDataWebSocketEvent");

  if(type == WS_EVT_CONNECT){

    Serial.println("Websocket client connection received");
    //client->text("Hello from ESP32 Server");

  } else if(type == WS_EVT_DISCONNECT){

    Serial.println("Client disconnected");

  }
}

void sendAllBeaconsOffToBeaconListeners() {

  // Clear the json object

  beaconJSONOut.clear();

  // Add the data to the JSON object

  beaconJSONOut["ACTION"] = "ALL_BEACONS_OFF";

  // Serialise it to the buffer

  char jsonString[1024];
  serializeJson(beaconJSONOut,jsonString);

  Serial.printf("*********** %s\n",jsonString);
  beaconDataWebSocket.textAll(jsonString);
}

void sendBeaconOnOffToBeaconListeners(char *beacon, bool onOff) {

  char cvtBuffer[100];

  // Clear the json object

  beaconJSONOut.clear();

  // Add the data to the JSON object

  beaconJSONOut["ACTION"] = "BEACON_ON_OFF";
  beaconJSONOut["BEACON"] = beacon;
  beaconJSONOut["ONOFF"] = true == onOff ? "1" : "0";

  // Serialise it to the buffer

  char jsonString[1024];
  serializeJson(beaconJSONOut,jsonString);

  Serial.printf("*********** %s\n",jsonString);
  beaconDataWebSocket.textAll(jsonString);
}

void sendBeaconColourToBeaconListeners(char *beacon, char *colour, char *subText) {

  char cvtBuffer[100];

  // Clear the json object

  beaconJSONOut.clear();

  // Add the data to the JSON object

  beaconJSONOut["ACTION"] = "BEACON_COLOUR";
  beaconJSONOut["BEACON"] = beacon;
  beaconJSONOut["COLOUR"] = colour;
  beaconJSONOut["SUBTEXT"] = subText;

  // Serialise it to the buffer

  char jsonString[1024];
  serializeJson(beaconJSONOut,jsonString);

  beaconDataWebSocket.textAll(jsonString);
}

void sendAllFrequenciesOffToBeaconListeners() {

  // Clear the json object

  beaconJSONOut.clear();

  // Add the data to the JSON object

  beaconJSONOut["ACTION"] = "ALL_FREQUENCIES_OFF";

  // Serialise it to the buffer

  char jsonString[1024];
  serializeJson(beaconJSONOut,jsonString);

  beaconDataWebSocket.textAll(jsonString);
}

void sendFrequencyActiveInActiveToBeaconListeners(double frequency, bool activeInActive) {

  char cvtBuffer[100];

  // Clear the json object

  beaconJSONOut.clear();

  // Add the data to the JSON object

  beaconJSONOut["ACTION"] = "FREQUENCY_ACTIVE_INACTIVE";

  sprintf(cvtBuffer,"%d",(int)(frequency * 1000));
  beaconJSONOut["FREQUENCY"] = cvtBuffer;

  beaconJSONOut["ACTIVEINACTIVE"] = true == activeInActive ? "1" : "0";

  // Serialise it to the buffer

  char jsonString[1024];
  serializeJson(beaconJSONOut,jsonString);

  beaconDataWebSocket.textAll(jsonString);
}

void sendFrequencyColourToBeaconListeners(double frequency, char *colour) {

  char cvtBuffer[100];

  // Clear the json object

  beaconJSONOut.clear();

  // Add the data to the JSON object

  beaconJSONOut["ACTION"] = "FREQUENCY_COLOUR";

  sprintf(cvtBuffer,"%d",(int)(frequency * 1000));
  beaconJSONOut["FREQUENCY"] = cvtBuffer;

  beaconJSONOut["COLOUR"] = colour;

  // Serialise it to the buffer

  char jsonString[1024];
  serializeJson(beaconJSONOut,jsonString);

  beaconDataWebSocket.textAll(jsonString);
}

void onRBNDataWebSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

  Serial.println("onRBNDataWebSocketEvent");

  if(type == WS_EVT_CONNECT){

    Serial.println("Websocket client connection received");
    //client->text("Hello from ESP32 Server");

  } else if(type == WS_EVT_DISCONNECT){

    Serial.println("Client disconnected");

  }
}

void sendToRBNDataListeners(char *spotter, char*spotted, double frequency, char *rbnType, char *rbnTime) {

  // Do we have space to queue up the next message?

  if (true == rbnDataWebSocket.availableForWriteAll()) {

    char cvtBuffer[100];

    // Clear the json object

    rbnJSONOut.clear();

    // Add the data to the JSON object

    rbnJSONOut["SPOTTER"] = spotter;
    rbnJSONOut["SPOTTED"] = spotted;
    rbnJSONOut["TYPE"] = rbnType;
    rbnJSONOut["TIME"] = rbnTime;

    sprintf(cvtBuffer,"%.03f MHz",frequency / 1000.0);
    rbnJSONOut["FREQUENCY"] = cvtBuffer;

    // Serialise it to the buffer

    char jsonString[1024];
    serializeJson(rbnJSONOut,jsonString);

    //Serial.printf("*********** %s\n",jsonString);
    rbnDataWebSocket.textAll(jsonString);
  }
}

extern void webServerSetUp(void _configurationUpdateHandler(), void _modeChangeHandler(bool manualEvent))
{

  // Save the configuration update handler and the mode change handler

  configurationUpdateHandler = _configurationUpdateHandler;
  modeChangeHandler = _modeChangeHandler;

  // Setup the websockets

  rbnDataWebSocket.onEvent(onBeaconDataWebSocketEvent);
  server.addHandler(&beaconDataWebSocket);

  rbnDataWebSocket.onEvent(onRBNDataWebSocketEvent);
  server.addHandler(&rbnDataWebSocket);

  // Setup the webserver

  server.on("/doReboot", HTTP_GET, onDoReboot);
  server.on("/getModeChange", HTTP_GET, onGetModeChange);
  server.on("/doSettingsUpdate", HTTP_GET, onDoSettingsUpdate);
  server.on("/getNameVersion", HTTP_GET, onGetNameVersion);
  server.on("/getSettingsData", HTTP_GET, onGetSettingsData);
  server.on("/getSSIDs", HTTP_GET, onGetSIDDs);
  server.on("/getUpTime", HTTP_GET, onGetUpTime);

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.begin();
}
