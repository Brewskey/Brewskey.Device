#ifndef Sensors_h
#define Sensors_h

#include "ISolenoids.h"
#include "ITick.h"
#include "ITap.h"
#include "Temperature.h"

class Sensors: public ISolenoids {
public:
  Sensors(ITap* taps, uint8_t tapCount);
  virtual void OpenSolenoids();
  virtual void CloseSolenoid(uint8_t solenoid);
  virtual void CloseSolenoids();
private:
  void SingleFlowCounter();

  Temperature* temperatureSensor;
  ITap* taps;

  uint8_t tapCount;
};

#endif
