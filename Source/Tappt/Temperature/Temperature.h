#ifndef Temperature_h
#define Temperature_h

#include "application.h"
#include "Pins.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "ITick.h"
#include "TapptTimer.h"

class Temperature : public ITick {
public:
  Temperature();
  virtual int Tick();
private:
  char json[64];
  void PrintAddress(DeviceAddress deviceAddress);

  TapptTimer timer = TapptTimer(120000);

  DallasTemperature sensors;

  // arrays to hold device address
  DeviceAddress insideThermometer;
};

#endif
