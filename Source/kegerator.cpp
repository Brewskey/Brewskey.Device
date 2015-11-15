#include "application.h"

//#define DEBUG 1

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
    led = new LED();
    led->IsBreathing(true);
    led->SetColor(255, 255, 255);
  
    Serial.begin(115200);


    nfcClient = new NfcClient(led);
    temperatureSensor = new Temperature();
    solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid);
    state = new KegeratorState(nfcClient, flowMeter, led);

  /*  while(!Serial.available()) {
      Spark.process();
    }
*/
    Serial.println("Starting");
}

void loop(void) {
  temperatureSensor->Tick();
  state->Tick();
}
