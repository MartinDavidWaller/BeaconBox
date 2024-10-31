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

extern time_t startUpTime;
extern struct Configuration configuration;

void onDoSettingsUpdate(AsyncWebServerRequest *request){

  AsyncWebParameter* hostnameInputParam = request->getParam("hostnameInput");
  AsyncWebParameter* passwordInputParam = request->getParam("passwordInput");
  AsyncWebParameter* ssidInputParam = request->getParam("ssidInput");
  AsyncWebParameter* callsignInputParam = request->getParam("callsignInput");
  AsyncWebParameter* spotterWildcardsInputParam = request->getParam("spotterWildcardsInput");
  AsyncWebParameter* spotterTimeOutMinutesInputParam = request->getParam("spotterTimeOutMinutesInput");
  AsyncWebParameter* frequencyStepTimeSecondsInputParam = request->getParam("frequencyStepTimeSecondsInput");
  AsyncWebParameter* ledBrightnessInputParam = request->getParam("ledBrightnessInput");

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
  response->printf("Copyright=\"[YEAR] M.D.Waller G0PJO. All rights reserved.\"");

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

void onGetSIDDs(AsyncWebServerRequest *request){

  char b[1024];
  b[0] = '\0';

  Serial.println("");
  Serial.println("Scanning for SSIDs");
  
  // We need to return a blob of XML containing the visible SSIDs
  
  strcpy(b,"<SSIDs>");

//**************

  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (int i = 0; i < n; ++i){
    // Add the SSID to the result
      
    strcat(b,"<SSID Name = \"");
    strcat(b,WiFi.SSID(i).c_str());
    strcat(b,"\" />");

    Serial.println("... " + WiFi.SSID(i));
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

  Serial.printf("*********** %s\n",jsonString);
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

  Serial.printf("*********** %s\n",jsonString);
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

  Serial.printf("*********** %s\n",jsonString);
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

  Serial.printf("*********** %s\n",jsonString);
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

void sendToRBNDataListeners(char *spotter, char*spotted, double frequency, char *rbnTime) {

  // Do we have space to queue up the next message?
  
  if (true == rbnDataWebSocket.availableForWriteAll()) {
  
    char cvtBuffer[100];
  
    // Clear the json object
  
    rbnJSONOut.clear();

    // Add the data to the JSON object
  
    rbnJSONOut["SPOTTER"] = spotter;
    rbnJSONOut["SPOTTED"] = spotted;
    rbnJSONOut["TIME"] = rbnTime;

    sprintf(cvtBuffer,"%.03f Mhz",frequency / 1000.0);
    rbnJSONOut["FREQUENCY"] = cvtBuffer;

    // Serialise it to the buffer

    char jsonString[1024];
    serializeJson(rbnJSONOut,jsonString);

    //Serial.printf("*********** %s\n",jsonString);
    rbnDataWebSocket.textAll(jsonString);
  }
}

extern void webServerSetUp(void _configurationUpdateHandler())
{ 

  // Save the configuration update handler
  
  configurationUpdateHandler = _configurationUpdateHandler;
  
  // Setup the websockets

  rbnDataWebSocket.onEvent(onBeaconDataWebSocketEvent);
  server.addHandler(&beaconDataWebSocket);
  
  rbnDataWebSocket.onEvent(onRBNDataWebSocketEvent);
  server.addHandler(&rbnDataWebSocket);
  
  // Setup the webserver

  server.on("/doSettingsUpdate", HTTP_GET, onDoSettingsUpdate);  
  server.on("/getNameVersion", HTTP_GET, onGetNameVersion);
  server.on("/getSettingsData", HTTP_GET, onGetSettingsData);
  server.on("/getSSIDs", HTTP_GET, onGetSIDDs);
  server.on("/getUpTime", HTTP_GET, onGetUpTime);
    
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  server.begin();
}
