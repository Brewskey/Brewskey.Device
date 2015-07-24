#ifndef Temperature_h
#define Temperature_h

#include "application.h"
#include "OneWire.h"
#include "ITick.h"

#ifndef TEMPERATURE_PIN
#define TEMPERATURE_PIN (D4)
#endif

class Temperature : public ITick {
public:
  virtual int Tick();
private:
};

#endif
