// remoteShutter.ino ~ Copyright 2017 Paul Beaudet ~ License MIT

#include "config.h" // include configuration file to hold wifi secrets we dont want in version control
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_FeatherOLED.h>
Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>
#define CAMERA_IP "192.168.54.1"
#define RELEASE_API_CALL "http://192.168.54.1/cam.cgi?mode=camcmd&value=capture"

#define BUTTON_A 0
#define BUTTON_B 16
#define BUTTON_C 2
#define SHUTTER_BUTTON 13
#define LED      0

void setup() {
    Serial.begin(115200);
    oled.init();
    buttonsSetup();
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);    // derived from config.h
    oled.println("remote shutter");
}

void loop() {
    byte action = buttonState();
    // buttonDebugging(action);
    if((WiFiMulti.run() == WL_CONNECTED)) {          // wait for WiFi connection
        if(action == 1){
            remoteShutter();
        }
    }
    oled.display();
}

void buttonDebugging(byte action){
    static byte lastAction = 0;

    if(action == lastAction){
        // Serial.print(".");
    } else {
        if(action == 1){
            oled.print("pressed");
        } else if (action == 2){
            oled.print("...");
        } else if (action == 0){
            oled.clearMsgArea();
        } else if (action == 3){
            oled.print("holding");
        } else if (action == 4){
            oled.print("...");
        }
        Serial.println(action, DEC);
    }
    lastAction = action;
}

void remoteShutter(){                            // this is probably blocking code
    static int shootsTaken = 0;

    HTTPClient http;                             // curious why this is nested here
    http.begin(RELEASE_API_CALL);                // HTTP request to camera to release shutter
    int httpCode = http.GET();                   // httpCode will be negative on error // BLOCKING? probably
    if(httpCode > 0) {                           // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if(httpCode == HTTP_CODE_OK) {           // file found at server
            String payload = http.getString();
            Serial.println(payload);
            shootsTaken++;
            oled.clearMsgArea();
            oled.println(shootsTaken);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void buttonsSetup(){
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
    pinMode(SHUTTER_BUTTON, INPUT_PULLUP);
}

// argument is assumed to be a press event
// checks for a debounced button press event
// 0 no pressing
// 1 just got pressed
// 2 still pressing
// 3 hold event
// 4 still holding
#define BOUNCETIME 20
#define HOLDSTATE 500
byte buttonState(){                           // remove default value to use in main sketch
    static unsigned long time = millis();
    static boolean bounceCheck = false;
    static boolean beingPressed = false;
    static boolean beingHeld = false;

    byte pressedState = 0;                    // low is a press with the pullup
    if(digitalRead(BUTTON_A) == LOW){         // if the button has been pressed
        if(bounceCheck) {                     // given timer has started
            if(millis() - time > BOUNCETIME){ // check if bounce time has elapesed
                if(beingPressed){
                    pressedState = 2;
                    if ( millis() - time > HOLDSTATE){ // case button held longer return state 2
                        if(beingHeld){
                            pressedState = 4;
                        } else {
                            pressedState = 3;           // signal hold state
                            beingHeld = true;
                        }
                    }
                } else {
                    pressedState = 1;                   // signal debounced press state
                    beingPressed = true;
                }
            }
        } else {
            bounceCheck = true; // note that the timing state is set
            time = millis();    // placemark when time press event started
        }
    } else {                // set all possible state back to zero
        bounceCheck = false;// in case the timing state was set, unset
        beingPressed = false;
        beingHeld = false;
    }
    return pressedState;
}
