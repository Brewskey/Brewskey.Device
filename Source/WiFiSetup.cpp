#include "WiFiSetup.h"


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
  String password = Serial.readStringUntil('\0');

  if (ssid.length() <= 0 || password.length() <= 0) {
    return;
  }

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
