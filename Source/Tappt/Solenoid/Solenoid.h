#ifndef Solenoid_h
#define Solenoid_h

#include "application.h"

#ifndef SOLENOID_PIN
#define SOLENOID_PIN (D2)
#endif

class Solenoid {
public:
  Solenoid();
  void Pour();
private:
  byte state, waitCount;
  volatile int flowCount = 0;
  unsigned long pourTimer, totalPulses;
};

#endif
