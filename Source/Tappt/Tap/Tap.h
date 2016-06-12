#ifndef Tap_h
#define Tap_h

#include "application.h"
#include "FlowMeter.h"
#include "IStateManager.h"
#include "ITick.h"
#include "Pins.h"
#include "Solenoid.h"

class Tap: public ITap {
public:
  Tap();
  void OpenValve();
  bool IsPouring();
  void Setup(IStateManager *kegeratorState);
  void AddToFlowCount(uint8_t pulses);
  void StopPour();
private:
  IStateManager* kegeratorState;
  FlowMeter* flowMeter;
  Solenoid* solenoid;

  Timer timer = Timer(5000, &Tap::StopPour, *this);

  bool isPouring;
  uint totalPulses;
};

#endif
