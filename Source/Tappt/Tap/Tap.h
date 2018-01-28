#ifndef Tap_h
#define Tap_h

#include "application.h"
#include "Tappt/Tap/ITap.h"
#include "Tappt/KegeratorState/IStateManager.h"
#include "Tappt/ITick.h"
#include "Tappt/Pins.h"

#define BEFORE_POUR_TIME_PERIOD 5000
#define AFTER_POUR_TIME_PERIOD 3000
#define PULSE_EPSILON 9

class Tap: public ITap {
public:
  Tap();
  uint32_t GetId();
  uint32_t GetPulsesPerGallon();
  bool IsPouring();
  uint32_t GetTotalPulses();
  void Setup(IStateManager *kegeratorState, uint32_t tapId, uint32_t pulsesPerGallon);
  virtual int Tick();
  virtual void SetAuthToken(String authenticationKey);
  virtual void AddToFlowCount(int32_t pulses);
  void StopPour();
private:
  IStateManager* kegeratorState;
  uint32_t tapId;
  int pulsesPerGallon;
  bool isPouring;
  uint32_t totalPulses;
  String authenticationKey;
  unsigned long pourStartTime;
};

#endif
