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

LED led;
KegeratorState* state;
Display* display;

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
}
