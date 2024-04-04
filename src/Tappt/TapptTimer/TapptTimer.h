#ifndef Timer_h
#define Timer_h

#include "Tappt/ITick.h"
#include "application.h"

class TapptTimer : public ITick {
 public:
  TapptTimer(unsigned long interval, unsigned long duration = 0);
  void Start();
  void Stop();
  virtual int Tick();
  void SetDuration(unsigned long duration = 0);

  bool IsRunning() { return this->isRunning; };
  bool ShouldTrigger() { return this->shouldTrigger; };

 private:
  bool isRunning = false;
  bool shouldTrigger = false;

  unsigned long duration;
  unsigned long interval;
  unsigned long previousMillis;
  unsigned long startMillis;
};

#endif
