#include "application.h"

#define DEBUG 100
//#define NDEF_DEBUG 1

//#define PLATFORM_ID 6

#include "Pins.h"
#include "FlowMeter.h"
#include "LED.h"
#include "KegeratorState.h"
#include "NfcClient.h"
#include "Solenoid.h"
#include "Temperature.h"
#include "WiFiSetup.h"

LED* led;
NfcClient* nfcClient;
FlowMeter* flowMeter;
Solenoid* solenoid;
Temperature* temperatureSensor;
KegeratorState* state;

void setup(void) {
    Serial.begin(9600);

    led = new LED();
    RGB.control(true);
    RGB.color(255, 255, 255);

    nfcClient = new NfcClient();
    temperatureSensor = new Temperature();
    solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid);
    state = new KegeratorState(nfcClient, flowMeter);

    while(!Serial.available()) {
      Spark.process();
    }
    
    Serial.println("Starting");
}

void loop(void) {
  temperatureSensor->Tick();
  state->Tick();
}
