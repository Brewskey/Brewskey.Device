#include "application.h"

//#define DEBUG 100
//#define NDEF_DEBUG 1

#include "Display.h"
#include "LED.h"
#include "KegeratorState.h"
#include "NfcClient.h"
#include "Pins.h"
#include "Sensors.h"
#include "ServerLink.h"
#include "TOTP.h"
#include "WiFiSetup.h"

LED* led = new LED();
NfcClient* nfcClient;
KegeratorState* state;
Display* display = new Display();

void setup(void) {
    Serial.begin(115200);

    RGB.control(true);
    RGB.color(255, 255, 255);

    Serial.println("Starting");

    /*while(!Serial.available()) {
      Spark.process();
    }*/

    nfcClient = new NfcClient();
    state = new KegeratorState(nfcClient, display);
}

void loop(void) {
  state->Tick();
}
