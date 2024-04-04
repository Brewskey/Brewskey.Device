#ifndef Tap_h
#define Tap_h

#include "Tappt/ITick.h"
#include "Tappt/KegeratorStateMachine/IKegeratorStateMachine.h"
#include "Tappt/Pins.h"
#include "Tappt/Tap/ITap.h"
#include "Tappt/Tap/TapConstraintType.h"
#include "Tappt/TapptTimer/TapptTimer.h"
#include "application.h"

#define PULSE_EPSILON 9

class Tap : public ITap {
 public:
  Tap();
  uint32_t GetId();
  uint32_t GetPulsesPerGallon();
  bool IsPouring();
  uint32_t GetTotalPulses();
  void Setup(IKegeratorStateMachine *kegeratorStateMachine, uint32_t tapId,
             uint32_t pulsesPerGallon, uint8_t timeForValveOpen);
  void SetConstraint(uint8_t tapConstraintType, uint32_t constraintPulses);
  virtual int Tick();
  virtual void SetAuthToken(String authenticationKey);
  virtual void SetTotalPulses(uint32_t pulses);
  void StopPour();

 private:
  IKegeratorStateMachine *kegeratorStateMachine;
  uint32_t tapId = 0;
  int pulsesPerGallon = 0;
  bool isPouring = false;
  uint32_t totalPulses = 0;
  String authenticationKey = "";
  unsigned long lastTimePulsesWasSet =
      0;  // is reset each time we get more pulses.
  unsigned long pourDeviceStartTimeInMillis =
      0;  // actual start time in UTC ticks
  uint8_t timeForValveOpen =
      0;  // Comes from settings -- how long the tap should wait before closing
  uint8_t tapConstraintType = TapConstraintType::NONE;
  uint32_t constraintPulses = 0;

  // Sometimes the expansion box and the brewskey box aren't quite in sync.
  // This makes sure we don't end up with duplicate pours.
  TapptTimer pourCooldownTimer = TapptTimer(500, 1000);
};

#endif
