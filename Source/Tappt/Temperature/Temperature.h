#ifndef Temperature_h
#define Temperature_h

#include "application.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "ITick.h"
#include "TapptTimer.h"

#ifndef TEMPERATURE_PIN
#define TEMPERATURE_PIN (D3)
#endif

class Temperature : public ITick {
public:
  Temperature();
  virtual int Tick();
private:
  char json[64];
  char coreID[30];
  void PrintAddress(DeviceAddress deviceAddress);

  TapptTimer timer = TapptTimer(10000);

  DallasTemperature sensors;

  // arrays to hold device address
  DeviceAddress insideThermometer;
};

#endif
