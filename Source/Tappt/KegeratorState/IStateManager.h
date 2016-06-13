#ifndef IStateManager_h
#define IStateManager_h

#include "DeviceSettings.h"
#include "ITap.h"
#include "application.h"

class IStateManager {
public:
  virtual void TapStartedPouring(ITap &tap) = 0;
  virtual void TapStoppedPouring(ITap &tap, uint totalPulses, String authenticationKey) = 0;
  virtual void Initialize(DeviceSettings *settings) = 0;
  virtual int StartPour(String data) = 0;
  virtual int Settings(String data) = 0;
  virtual ~IStateManager() {}
};

#endif
