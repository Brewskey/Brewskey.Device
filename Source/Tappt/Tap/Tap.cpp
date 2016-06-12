#include "Tap.h"

#define PULSE_EPSILON 3

Tap::Tap() {
}

void Tap::OpenValve() {
  // TODO - open solenoid
}

bool Tap::IsPouring() {
  return this->isPouring;
}

void Tap::Setup(IStateManager *kegeratorState) {
  this->kegeratorState = kegeratorState;
}

void Tap::StopPour() {
  if (this->totalPulses > PULSE_EPSILON) {
    // TODO - send pour to server
  }

  this->totalPulses = 0;
  this->timer.stop();
  this->timer.reset();
}

void Tap::AddToFlowCount(uint8_t pulses) {
  this->isPouring = true;

  if (this->totalPulses == 0) {
    this->timer.start();
  } else {
    this->timer.reset();
  }

  this->totalPulses += pulses;
  this->kegeratorState->TapIsPouring(*this);
}
