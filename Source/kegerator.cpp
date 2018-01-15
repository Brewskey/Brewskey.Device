#include "application.h"

//#define DEBUG 100
//#define NDEF_DEBUG 1
#include "Tappt/Pins.h"
#include "Tappt/Display/Display.h"
#include "Tappt/led/LED.h"
#include "Tappt/KegeratorState/KegeratorState.h"
#include "Tappt/NfcClient/NfcClient.h"
#include "Tappt/Sensors/Sensors.h"
#include "Tappt/ServerLink/ServerLink.h"
#include "Tappt/TapptTimer/TapptTimer.h"
#include "TOTP/TOTP.h"

PRODUCT_ID(BREWSKEY_PRODUCT_ID);
PRODUCT_VERSION(BREWSKEY_PRODUCT_VERSION);

#define MILLISECONDS_IN_DAY 86400000

LED led;
KegeratorState* state;
Display* display;
TapptTimer timeSync = TapptTimer(MILLISECONDS_IN_DAY);

void setup(void) {
    RGB.control(true);
    RGB.color(255, 255, 255);
    display = new Display();
    Serial.begin(115200);
    Serial1.begin(19200);

    Serial.println("Starting");
/*
    while(!Serial.available()) {
      Spark.process();
    }
*/

    state = new KegeratorState(display);
}

void loop(void) {
  state->Tick();
  timeSync.Tick();

  if (timeSync.ShouldTrigger()) {
    Particle.syncTime();
  }
}
