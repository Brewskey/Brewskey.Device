#ifndef Solenoid_h
#define Solenoid_h

#include "application.h"
#include "ITick.h"

#ifndef SOLENOID_PIN
#define SOLENOID_PIN (D2)
#endif

class Solenoid : public ITick {
public:
  Solenoid();
  virtual int Tick();
private:
  byte state, waitCount;
  volatile int flowCount = 0;
  unsigned long pourTimer, totalPulses;
};

#endif
