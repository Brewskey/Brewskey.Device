#ifndef ITap_h
#define ITap_h

#include "application.h"

class ITap {
public:
  virtual uint32_t GetId() = 0;
  virtual void AddToFlowCount(uint pulses) = 0;
  virtual uint GetTotalPulses() = 0;
  virtual bool IsPouring() = 0;
  virtual void SetAuthToken(String authenticationKey) = 0;
  virtual ~ITap() {}
};

#endif
