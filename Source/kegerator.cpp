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
int state = KegeratorState::LISTENING;

void setup(void) {
    Serial.begin(115200);

    led = new LED();
    nfcClient = new NfcClient(led);
    temperatureSensor = new Temperature();
    solenoid = new Solenoid();
    flowMeter = new FlowMeter(solenoid, led);

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
    nfcClient->Tick();
  }
}

/*
void setup()
{
  Serial.begin(115200);

  led = new LED();
  led->SetColor(0, 255, 47);
  led->IsBreathing(true);
}

void loop()
{
  led->Tick();
}
*/
