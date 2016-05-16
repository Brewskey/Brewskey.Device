#include "application.h"

//#define DEBUG 100
//#define NDEF_DEBUG 1

#include "Display.h"
#include "Pins.h"
#include "FlowMeter.h"
#include "LED.h"
#include "KegeratorState.h"
#include "NfcClient.h"
#include "Solenoid.h"
#include "TOTP.h"
#include "Temperature.h"
#include "WiFiSetup.h"

LED* led = new LED();;
NfcClient* nfcClient;
FlowMeter* flowMeter;
Solenoid* solenoid;
Temperature* temperatureSensor;
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
    temperatureSensor = new Temperature();
    solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid, display);
    state = new KegeratorState(nfcClient, flowMeter, solenoid, display);
}

void loop(void) {
  temperatureSensor->Tick();
  state->Tick();
}
