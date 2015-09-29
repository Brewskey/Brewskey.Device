#ifndef FlowMeter_h
#define FlowMeter_h

#include "Solenoid.h"

#ifndef FLOW_PIN
#define FLOW_PIN (D2)
#endif

#define PULSE_EPSILON 0

class FlowMeter : public ITick {
public:
  FlowMeter(Solenoid *solenoid);
  void StartPour();
  void StopPour();
  virtual int Tick();
private:
  Solenoid *solenoid;
  bool pouring;
  byte state, waitCount;
  int lastFlowCount = 0;
  unsigned long pourTimer;

  void FlowCounter();
};

#endif
