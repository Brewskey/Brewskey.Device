#ifndef Tap_h
#define Tap_h

#include "application.h"
#include "Tappt/Tap/ITap.h"
#include "Tappt/KegeratorStateMachine/IKegeratorStateMachine.h"
#include "Tappt/ITick.h"
#include "Tappt/Pins.h"
#include "Tappt/TapptTimer/TapptTimer.h"

#define BEFORE_POUR_TIME_PERIOD 5000
#define AFTER_POUR_TIME_PERIOD 3000
#define PULSE_EPSILON 9

class Tap : public ITap {
public:
  Tap();
  uint32_t GetId();
  uint32_t GetPulsesPerGallon();
  bool IsPouring();
  uint32_t GetTotalPulses();
  void Setup(IKegeratorStateMachine *kegeratorStateMachine, uint32_t tapId, uint32_t pulsesPerGallon);
  virtual int Tick();
  virtual void SetAuthToken(String authenticationKey);
  virtual void SetTotalPulses(uint32_t pulses);
  void StopPour();
private:
  IKegeratorStateMachine * kegeratorStateMachine;
  uint32_t tapId;
  int pulsesPerGallon;
  bool isPouring;
  uint32_t totalPulses;
  String authenticationKey;
  unsigned long pourStartTime;  // is reset each time we get more pulses.
  unsigned long pourDeviceEndTime; // actual start time in UTC ticks
  unsigned long pourDeviceStartTime; // actual start time in UTC ticks

  // Sometimes the expansion box and the brewskey box aren't quite in sync.
  // This makes sure we don't end up with duplicate pours.
  TapptTimer pourCooldownTimer = TapptTimer(500, 400);
};

#endif
