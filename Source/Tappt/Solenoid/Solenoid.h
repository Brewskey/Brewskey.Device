#ifndef Solenoid_h
#define Solenoid_h

#include "Rest.h"

#ifndef SOLENOID_PIN
#define SOLENOID_PIN (D2)
#endif

class Solenoid {
public:
  Solenoid();
  void Pour();
private:
  Rest* restClient;

  byte state, waitCount;
  volatile int flowCount = 0;
  unsigned long pourTimer, totalPulses;
};

#endif
