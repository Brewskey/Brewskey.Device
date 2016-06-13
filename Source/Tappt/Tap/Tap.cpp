#include "Tap.h"

#define PULSE_EPSILON 3

Tap::Tap() {
  this->timer.stop();
}

String Tap::GetId() {
  return this->tapId;
}

bool Tap::IsPouring() {
  return this->isPouring;
}

void Tap::Setup(IStateManager *kegeratorState) {
  this->kegeratorState = kegeratorState;
}

void Tap::SetId(String tapId) {
  this->tapId = tapId;
}

void Tap::SetAuthToken(String authenticationKey) {
  this->authenticationKey = authenticationKey;
}

void Tap::StopPour() {
  this->isPouring = false;
  this->kegeratorState->TapStoppedPouring(
    *this,
    this->totalPulses,
    this->authenticationKey
  );

  this->totalPulses = 0;
  this->authenticationKey = "";
  //this->timer.changePeriod(BEFORE_POUR_TIME_PERIOD);
  this->timer.stop();
}

void Tap::AddToFlowCount(uint8_t pulses) {
  this->totalPulses += pulses;

  if (this->totalPulses > PULSE_EPSILON && !this->isPouring) {
    //this->timer.changePeriod(AFTER_POUR_TIME_PERIOD);
    this->timer.start();
    this->isPouring = true;
    this->kegeratorState->TapStartedPouring(*this);
  } else {
    this->timer.reset();
  }
}
