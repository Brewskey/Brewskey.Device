#include "application.h"

//#define DEBUG 1

//#define PLATFORM_ID 6

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

    nfcClient = new NfcClient();
    temperatureSensor = new Temperature();
    solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid);

    //while(!Serial.available()) {
      //Spark.process();
    //}

    Serial.println("Starting");
}

void loop(void) {
  temperatureSensor->Tick();

  int isPouring = flowMeter->Tick();

  // if isPouring equals 1 then it is currently pouring and shouldn't accept
  // nfc
  if (isPouring <= 0) {
    nfcClient->Tick();
  }
}
