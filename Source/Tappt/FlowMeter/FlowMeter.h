#ifndef FlowMeter_h
#define FlowMeter_h

#include "Pins.h"
#include "LED.h"
#include "Solenoid.h"
#include "TapptTimer.h"

#define PULSE_EPSILON 1

class FlowMeter : public ITick {
public:
  FlowMeter(Solenoid *solenoid, LED* led);
  int StartPour(String data);
  void StopPour();
  virtual int Tick();
private:
  Solenoid *solenoid;
  LED *led;
  bool pouring = false;
  byte state, waitCount;
  volatile int flowCount = 0;
  unsigned long lastFlowCount = 0;
  TapptTimer timer = TapptTimer(1000);

  char json[76];
  String pourKey;

  void FlowCounter();
};

#endif
