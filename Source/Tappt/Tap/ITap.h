#ifndef ITap_h
#define ITap_h

#include "application.h"

class ITap {
public:
  virtual void StopPour() = 0;
  virtual void AddToFlowCount(uint8_t pulses) = 0;
  virtual ~ITap() {}
};

#endif
