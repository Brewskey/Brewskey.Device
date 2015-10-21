#include "TapptTimer.h"

TapptTimer::TapptTimer(unsigned long interval) {
  this->interval = interval;
  this->Reset();
}

void TapptTimer::Reset() {
  this->ShouldTrigger = false;
  this->previousMillis = millis();
}

int TapptTimer::Tick() {
  unsigned long currentMillis = millis();
  this->ShouldTrigger = currentMillis - this->previousMillis > this->interval;

  if (this->ShouldTrigger)
  {
    this->previousMillis = currentMillis;
  }
}
