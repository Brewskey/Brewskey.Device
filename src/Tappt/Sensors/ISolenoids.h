#ifndef ISolenoids_h
#define ISolenoids_h

#include "application.h"
#include "Tappt/KegeratorStateMachine/KegeratorState.h"
#include "Tappt/ITick.h"


class ISolenoids : public ITick {
public:
  virtual void OpenSolenoids() = 0;
  virtual void CloseSolenoid(uint8_t solenoid) = 0;
  virtual void CloseSolenoids() = 0;
  virtual void ResetFlowSensor(uint8_t solenoid) = 0;
  virtual void SetState(KegeratorState::e state) = 0;
  virtual ~ISolenoids() {}
};

#endif
