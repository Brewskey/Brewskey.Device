#include "TapptTimer.h"

TapptTimer::TapptTimer(unsigned long interval, unsigned long duration) {
  this->interval = interval;
  this->duration = duration;
}

void TapptTimer::Start() {
  this->isRunning = true;
  this->shouldTrigger = true;
  this->previousMillis = millis();
  this->startMillis = millis();
}

void TapptTimer::Stop() { this->isRunning = false; }

int TapptTimer::Tick() {
  if (!this->isRunning) {
    return 0;
  }

  unsigned long currentMillis = millis();

  if (this->duration != 0) {
    if ((long)(currentMillis - this->startMillis) > this->duration) {
      this->isRunning = false;
      return 0;
    }
  }

  if (this->interval != 0) {
    this->shouldTrigger =
        (long)(currentMillis - this->previousMillis) > this->interval;

    if (this->shouldTrigger) {
      this->previousMillis = currentMillis;
    }
  }

  return 0;
}

void TapptTimer::SetDuration(unsigned long duration) {
  this->duration = duration;
}
