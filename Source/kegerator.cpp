#include "application.h"

//#define DEBUG 1

//#define PLATFORM_ID 6

#include "WiFiSetup.h"
#include "KegeratorState.h"
#include "NfcClient.h"
#include "Temperature.h"
#include "Solenoid.h"

#define SCK  (A3)
#define MOSI (A5)
#define SS   (A2)
#define MISO (A4)

#define FLOW (A7)

NfcClient* nfcClient;
Temperature* temperatureSensor;
Solenoid* solenoid;
int state = KegeratorState::LISTENING;

int flowCount = 0;
void flowCounter()
{
	flowCount++;
}

void setup(void) {
    Serial.begin(115200);

    //WifiSetup();

    while(!Serial.available()) {
      Spark.process();
    }

    nfcClient = new NfcClient();
    temperatureSensor = new Temperature();
		solenoid = new Solenoid();

    attachInterrupt(FLOW, flowCounter, FALLING);
}

void loop(void) {
  //WiFiListen();
  //return;
  temperatureSensor->Tick();

  Serial.print("flow: ");Serial.println(flowCount);

  switch (state) {
    case KegeratorState::LISTENING:
      {
        nfcClient->Tick();
      }
      break;
    case KegeratorState::POURING:
      break;
    case KegeratorState::DONE_POURING:
      break;
  }
}
