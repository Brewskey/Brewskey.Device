#include "Tap.h"

Tap::Tap() {
  this->tapId = 0;
  this->pulsesPerGallon = 0;
  this->isPouring = false;
  this->totalPulses = 0;
  this->pourStartTime = 0;
}

uint32_t Tap::GetId() {
  return this->tapId;
}

uint32_t Tap::GetPulsesPerGallon() {
  return this->pulsesPerGallon;
}

uint32_t Tap::GetTotalPulses() {
  return this->totalPulses;
}

bool Tap::IsPouring() {
  return this->isPouring;
}
void Tap::Setup(
  IStateManager *kegeratorState,
  uint32_t tapId,
  uint32_t pulsesPerGallon
) {
  this->kegeratorState = kegeratorState;
  this->tapId = tapId;
  this->pulsesPerGallon = pulsesPerGallon;
}

void Tap::SetAuthToken(String authenticationKey) {
  this->authenticationKey = authenticationKey;
}

void Tap::StopPour() {
  Serial.println("Tap::StopPour");
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

void Tap::AddToFlowCount(int32_t pulses) {
  // Don't start pouring if pulses aren't being sent.
  // TODO - we need to find a good way to filter our phantom pours.  Maybe do
  // an average over time and kill it that way.
  if (pulses <= 0) {
    return;
  }

  this->totalPulses += pulses;
  this->pourStartTime = millis();

  if (this->totalPulses > PULSE_EPSILON && !this->isPouring) {
    this->isPouring = true;

    this->kegeratorState->TapStartedPouring(*this);
  }
}
