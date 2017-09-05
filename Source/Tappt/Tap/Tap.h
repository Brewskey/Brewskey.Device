#ifndef Tap_h
#define Tap_h

#include "application.h"
#include "Tappt/Tap/ITap.h"
#include "Tappt/KegeratorState/IStateManager.h"
#include "Tappt/ITick.h"
#include "Tappt/Pins.h"

#define BEFORE_POUR_TIME_PERIOD 5000
#define AFTER_POUR_TIME_PERIOD 3000

class Tap: public ITap, public ITick {
public:
  Tap();
  String GetId();
  int GetPulsesPerGallon();
  bool IsPouring();
  uint GetTotalPulses();
  void Setup(IStateManager *kegeratorState, String tapId, int pulsesPerGallon);
  virtual int Tick();
  virtual void SetAuthToken(String authenticationKey);
  virtual void AddToFlowCount(uint pulses);
  void StopPour();
private:
  IStateManager* kegeratorState;
  String tapId;
  int pulsesPerGallon;
  bool isPouring;
  uint totalPulses;
  String authenticationKey;
  unsigned long pourStartTime;
};

#endif
