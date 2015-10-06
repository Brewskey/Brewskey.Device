#ifndef FlowMeter_h
#define FlowMeter_h

#include "Solenoid.h"
#include "Timer.h"

#ifndef FLOW_PIN
#define FLOW_PIN (D2)
#endif

#define PULSE_EPSILON 0

class FlowMeter : public ITick {
public:
  FlowMeter(Solenoid *solenoid);
  int StartPour(String parameters);
  void StopPour();
  virtual int Tick();
private:
  Solenoid *solenoid;
  bool pouring;
  byte state, waitCount;
  volatile int flowCount = 0;
  unsigned long lastFlowCount = 0;
  Timer timer = Timer(1000);

  char json[128];
  String pourKey;

  void FlowCounter();
};

#endif
