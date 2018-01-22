#ifndef IStateManager_h
#define IStateManager_h

#include "application.h"
#include "Tappt/ServerLink/DeviceSettings.h"
#include "Tappt/Tap/ITap.h"
#include "Tappt/ITick.h"

class IStateManager: public ITick {
public:
  virtual void TapStartedPouring(ITap &tap) = 0;
  virtual void TapStoppedPouring(ITap &tap, uint32_t totalPulses, String authenticationKey) = 0;
  virtual void Initialize(DeviceSettings *settings) = 0;
  virtual int StartPour(String data) = 0;
  virtual int Settings(String data) = 0;
  virtual ~IStateManager() {}
};

#endif
