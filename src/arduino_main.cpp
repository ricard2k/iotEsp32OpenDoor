/**
   Example for the ESP32 HTTP(S) Webserver

   IMPORTANT NOTE:
   To run this script, your need to
    1) Enter your WiFi SSID and PSK below this comment
    2) Make sure to have certificate data available. You will find a
       shell script and instructions to do so in the library folder
       under extras/

   This script will install an HTTPS Server on your ESP32 with the following
   functionalities:
    - Show simple page on web server root
    - 404 for everything else
*/

#include "Arduino.h"

#include "relay_http.h"
#include "esp32WifiManager.h"

void setup() {

  //Define relay pins
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);

  // For logging
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.resetSettings();

  //Connect to Wifi
  if(!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  } 

  setupRelayHttpServer();
}

void loop() {


  loopRelayHttpServer();


  if (getPreviousMillis() < millis()) {
    digitalWrite(RELAY_1, LOW);
    digitalWrite(RELAY_2, LOW);
  }

  delay(1);
}

