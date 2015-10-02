#include "application.h"

//#define DEBUG 1

//#define PLATFORM_ID 6
#define NFC 0

#include "FlowMeter.h"
#include "KegeratorState.h"
#include "NfcClient.h"
#include "Solenoid.h"
#include "Temperature.h"
#include "WiFiSetup.h"

#define SCK  (A3)
#define MOSI (A5)
#define SS   (A2)
#define MISO (A4)

NfcClient* nfcClient;
FlowMeter* flowMeter;
Solenoid* solenoid;
Temperature* temperatureSensor;
int state = KegeratorState::LISTENING;

void setup(void) {
    Serial.begin(115200);
    //WifiSetup();

    while(!Serial.available()) {
      Spark.process();
    }

    Serial.println("Starting");

#if NFC == 1
    nfcClient = new NfcClient();
#endif
    temperatureSensor = new Temperature();
		solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid);
}

void loop(void) {
  temperatureSensor->Tick();

  switch (state) {
    case KegeratorState::LISTENING:
      {
        #if NFC == 1
        nfcClient->Tick();
        #endif
      }
      break;
    case KegeratorState::POURING:
      break;
    case KegeratorState::DONE_POURING:
      break;
  }
}
