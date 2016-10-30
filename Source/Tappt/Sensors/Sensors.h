#ifndef Sensors_h
#define Sensors_h

#include "ISolenoids.h"
#include "ITick.h"
#include "Tap.h"
#include "Temperature.h"

class Sensors: public ISolenoids, public ITick {
public:
  Sensors(Tap* taps, uint8_t tapCount);
  virtual int Tick();
  virtual void OpenSolenoids();
  virtual void CloseSolenoid(uint8_t solenoid);
  virtual void CloseSolenoids();
private:
  void SingleFlowCounter();

  Temperature* temperatureSensor;
  Tap* taps;

  uint8_t tapCount;
};

#endif
