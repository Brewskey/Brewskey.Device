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
  IKegeratorStateMachine *kegeratorStateMachine,
  uint32_t tapId,
  uint32_t pulsesPerGallon
) {
  this->kegeratorStateMachine = kegeratorStateMachine;
  this->tapId = tapId;
  this->pulsesPerGallon = pulsesPerGallon;
}

void Tap::SetAuthToken(String authenticationKey) {
  this->authenticationKey = authenticationKey;
}

void Tap::StopPour() {
  this->pourCooldownTimer.Start();

  bool isPouring = this->isPouring;
  this->isPouring = false;


  uint32_t totalPulses = this->totalPulses;
  String authenticationKey = this->authenticationKey;

  this->totalPulses = 0;
  this->authenticationKey = "";

  if (totalPulses > PULSE_EPSILON && isPouring) {
    this->kegeratorStateMachine->TapStoppedPouring(
      this->tapId,
      totalPulses,
      authenticationKey,
      this->pourDeviceStartTime,
      this->pourDeviceEndTime
    );
  }
}

int Tap::Tick() {
  // handle isPouring here :)
  this->pourCooldownTimer.Tick();
  if (!this->isPouring) {
    return 0;
  }

  long delta = millis() - this->pourStartTime;
  if (delta > 5000 && delta != 4294967295) {
    this->StopPour();
  }

  return 0;
}

void Tap::SetTotalPulses(uint32_t pulses) {
  // If the pour cooldown is running, wait to set pulses.
  if (this->pourCooldownTimer.IsRunning()) {
    return;
  };

  // Don't start pouring if pulses aren't being sent.
  // TODO - we need to find a good way to filter our phantom pours.  Maybe do
  // an average over time and kill it that way.
  if (pulses == 0) {
    return;
  }

  this->totalPulses = pulses;
  this->pourStartTime = millis();
  this->pourDeviceEndTime = Time.now();

  if (this->totalPulses > PULSE_EPSILON && !this->isPouring) {
    this->isPouring = true;
    this->pourDeviceStartTime = Time.now();

    this->kegeratorStateMachine->TapStartedPouring(*this);
  }
}
