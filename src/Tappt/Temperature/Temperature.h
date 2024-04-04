#ifndef Temperature_h
#define Temperature_h

#include "DallasTemperature/DallasTemperature.h"
#include "OneWire/OneWire.h"
#include "Tappt/ITick.h"
#include "Tappt/Pins.h"
#include "Tappt/TapptTimer/TapptTimer.h"
#include "application.h"

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
