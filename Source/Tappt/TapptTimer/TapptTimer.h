#ifndef Timer_h
#define Timer_h

#include "application.h"
#include "ITick.h"

class TapptTimer: public ITick {
public:
  TapptTimer(unsigned long interval);
  void Reset();
  virtual int Tick();

  bool ShouldTrigger = false;
private:
  unsigned long interval;
  unsigned long previousMillis;
};

#endif
