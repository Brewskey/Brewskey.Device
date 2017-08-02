#include "Tap.h"

#define PULSE_EPSILON 10


Tap::Tap() {
  this->totalPulses = 0;
}

String Tap::GetId() {
  return this->tapId;
}

int Tap::GetPulsesPerGallon() {
  return this->pulsesPerGallon;
}

uint Tap::GetTotalPulses() {
  return this->totalPulses;
}

bool Tap::IsPouring() {
  return this->isPouring;
}
void Tap::Setup(
  IStateManager *kegeratorState,
  String tapId,
  int pulsesPerGallon
) {
  this->kegeratorState = kegeratorState;
  this->tapId = tapId;
  this->pulsesPerGallon = pulsesPerGallon;
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

  long delta = millis() - this->pourStartTime;
  if (delta > 5000 && delta != 4294967295) {
    this->StopPour();
  }

  return 0;
}

void Tap::AddToFlowCount(uint pulses) {
  // Don't start pouring if pulses aren't being sent.
  // We use 5 here to filter out the phantom pours
  if (pulses <= 5) {
    return;
  }

  this->totalPulses += pulses;
  this->pourStartTime = millis();

  if (this->totalPulses > PULSE_EPSILON && !this->isPouring) {
    this->isPouring = true;

    this->kegeratorState->TapStartedPouring(*this);
  }
}
