// remoteShutter.ino ~ Copyright 2017 Paul Beaudet ~ License MIT

#include "config.h" // include configuration file to hold wifi secrets we dont want in version control
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
#define CAMERA_IP "192.168.54.1"
#define RELEASE_API_CALL "http://192.168.54.1/cam.cgi?mode=camcmd&value=capture"

#define BUTTON_A 0
#define BUTTON_B 16
#define BUTTON_C 2
#define LED      0

void setup() {
    USE_SERIAL.begin(115200);
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);    // derived from config.h
}

void loop() {
    if((WiFiMulti.run() == WL_CONNECTED)) {          // wait for WiFi connection
        if(!digitalRead(BUTTON_A)){
            remoteShutter();
        }
        delay(100);
    }
}

void remoteShutter(){
    HTTPClient http;                             // curious why this is nested here
    USE_SERIAL.print("[HTTP] begin...\n");       // configure traged server and url
    http.begin(RELEASE_API_CALL);                // HTTP request to camera to release shutter
    USE_SERIAL.print("[HTTP] GET...\n");         // start connection and send HTTP header
    int httpCode = http.GET();                   // httpCode will be negative on error
    if(httpCode > 0) {                           // HTTP header has been send and Server response header has been handled
        USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        if(httpCode == HTTP_CODE_OK) {           // file found at server
            String payload = http.getString();
            USE_SERIAL.println(payload);
        }
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}
