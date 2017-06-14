// remoteShutter.ino ~ Copyright 2017 Paul Beaudet ~ License MIT

#include "config.h" // include configuration file to hold wifi secrets we dont want in version control
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

void setup() {
    USE_SERIAL.begin(115200);
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);    // derived from config.h
}

void loop() {
    if((WiFiMulti.run() == WL_CONNECTED)) {          // wait for WiFi connection
        HTTPClient http;                             // curious why this is nested here
        USE_SERIAL.print("[HTTP] begin...\n");       // configure traged server and url
        //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
        http.begin("http://192.168.1.12/test.html"); // HTTP
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
    delay(10000);
}
