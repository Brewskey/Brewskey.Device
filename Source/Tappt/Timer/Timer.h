#ifndef Timer_h
#define Timer_h

#include "application.h"
#include "ITick.h"

class Timer: public ITick {
public:
  Timer(unsigned long interval);
  void Reset();
  virtual int Tick();

  bool ShouldTrigger = false;
private:
  unsigned long interval;
  unsigned long previousMillis;
};

#endif
