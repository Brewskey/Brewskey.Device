#ifndef Solenoid_h
#define Solenoid_h

#include "application.h"
#include "ITick.h"

#ifndef SOLENOID_PIN
#define SOLENOID_PIN (D0)
#endif

class Solenoid {
public:
  Solenoid();
  void Open();
  void Close();
};

#endif
