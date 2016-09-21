#include "Tap.h"

#define PULSE_EPSILON 3

Tap::Tap() {
  this->totalPulses = 0;
}

String Tap::GetId() {
  return this->tapId;
}

uint Tap::GetTotalPulses() {
  return this->totalPulses;
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
  bool isPouring = this->isPouring;
  this->isPouring = false;
  if (this->totalPulses > PULSE_EPSILON && isPouring) {
    this->kegeratorState->TapStoppedPouring(
      *this,
      this->totalPulses,
      this->authenticationKey
    );
  }

  this->totalPulses = 0;
  this->authenticationKey = "";
}

int Tap::Tick() {
  // handle isPouring here :)
  if (!this->isPouring) {
    return 0;
  }

  unsigned long delta = millis() - this->pourStartTime;

  if (delta > 5000) {
    this->StopPour();
  }

  return 0;
}

void Tap::AddToFlowCount(uint pulses) {
  this->totalPulses += pulses;
  this->pourStartTime = millis();

  if (this->totalPulses > PULSE_EPSILON && !this->isPouring) {
    this->isPouring = true;
  }
}
