#ifndef WifiSetup_h
#define WifiSetup_h

#include "application.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

void WiFiListen();

void WifiSetup() {
  WiFi.on();
  WiFi.connect();

  Spark.connect();

  while (!Spark.connected()) {
    Spark.process();
    WiFiListen();
  }
}

void WiFiListen() {
  int bytes = Serial.available();
  if (!bytes) {
    return;
  }

  int auth = Serial.read();
  Serial.read(); // empty bit
  String ssid = Serial.readStringUntil('\0');
  Serial.println(ssid);Serial.println();
  String password = Serial.readStringUntil('\0');

  Spark.disconnect();
  WiFi.off();
  WiFi.on();
  WiFi.clearCredentials();   // if you only want one set of credentials stored
  WiFi.setCredentials(ssid, password, auth);

  WiFi.connect();

  while(WiFi.connecting()) {
    Spark.process();
  }

  Spark.connect();

  Serial.println("WiFiSetupDone");
}

#endif
