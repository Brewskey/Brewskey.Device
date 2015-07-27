#ifndef Temperature_h
#define Temperature_h

#include "application.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "ITick.h"

#ifndef TEMPERATURE_PIN
#define TEMPERATURE_PIN (D2)
#endif

class Temperature : public ITick {
public:
  Temperature();
  virtual int Tick();
private:
  void PrintAddress(DeviceAddress deviceAddress);

  DallasTemperature sensors;

  // arrays to hold device address
  DeviceAddress insideThermometer;
};

#endif
