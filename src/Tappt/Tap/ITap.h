#ifndef ITap_h
#define ITap_h

#include "Tappt/ITick.h"
#include "application.h"

class ITap : public ITick {
 public:
  virtual uint32_t GetId() = 0;
  virtual void SetTotalPulses(uint32_t pulses) = 0;
  virtual uint32_t GetTotalPulses() = 0;
  virtual bool IsPouring() = 0;
  virtual void SetAuthToken(String authenticationKey) = 0;
  virtual ~ITap() {}
};

#endif
