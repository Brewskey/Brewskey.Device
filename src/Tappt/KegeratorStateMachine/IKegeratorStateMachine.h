#ifndef IStateManager_h
#define IStateManager_h

#include "application.h"
#include "Tappt/ServerLink/DeviceSettings.h"
#include "Tappt/Tap/ITap.h"
#include "Tappt/Tap/TapConstraint.h"
#include "Tappt/ITick.h"

class IKegeratorStateMachine : public ITick {
public:
  virtual void TapStartedPouring(ITap &tap) = 0;
  virtual void TapStoppedPouring(uint32_t tapID, uint32_t totalPulses, String authenticationKey, uint32_t pourStartTime, uint32_t pourEndTime) = 0;
  virtual void Initialize(DeviceSettings *settings) = 0;
  virtual int StartPour(String token, int constraintCount, TapConstraint *constraints) = 0;
  virtual int Settings(String data) = 0;
  virtual void OnConfigureNextBox(uint8_t destination) = 0;
  virtual ~IKegeratorStateMachine() {}
};

#endif
