#ifndef Tap_h
#define Tap_h

#include "application.h"
#include "IStateManager.h"
#include "ITick.h"
#include "Pins.h"

#define BEFORE_POUR_TIME_PERIOD 5000
#define AFTER_POUR_TIME_PERIOD 3000

class Tap: public ITap {
public:
  Tap();
  String GetId();
  bool IsPouring();
  uint GetTotalPulses();
  void Setup(IStateManager *kegeratorState);
  void SetId(String tapId);
  virtual void SetAuthToken(String authenticationKey);
  virtual void AddToFlowCount(uint8_t pulses);
  void StopPour();
private:
  IStateManager* kegeratorState;

  Timer timer = Timer(BEFORE_POUR_TIME_PERIOD, &Tap::StopPour, *this, true);

  String tapId;
  bool isPouring;
  uint totalPulses;
  String authenticationKey;
};

#endif
