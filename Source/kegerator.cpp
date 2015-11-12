#include "application.h"

//#define DEBUG 1

//#define PLATFORM_ID 6
//#define SPI_HW_MODE 1
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
    Serial.begin(115200);

    led = new LED();
    nfcClient = new NfcClient(led);
    temperatureSensor = new Temperature();
    solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid, led);
    state = new KegeratorState(nfcClient, flowMeter);

    led->SetColor(255, 255, 255);

  /*  while(!Serial.available()) {
      Spark.process();
    }
*/
    Serial.println("Starting");
}

void loop(void) {
  temperatureSensor->Tick();
  led->Tick();

  int isPouring = flowMeter->Tick();

  // if isPouring equals 1 then it is currently pouring and shouldn't accept
  // nfc
  if (isPouring <= 0) {
    NfcState::value nfcState = (NfcState::value)nfcClient->Tick();
    state->SetNfcState(nfcState);
  }
}
