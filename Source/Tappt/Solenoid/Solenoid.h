#ifndef Solenoid_h
#define Solenoid_h

#include "Pins.h"
#include "application.h"
#include "ITick.h"

class Solenoid {
public:
  Solenoid();
  void Open();
  void Close();
};

#endif
