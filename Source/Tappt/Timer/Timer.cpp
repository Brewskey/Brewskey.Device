#include "Timer.h"

Timer::Timer(unsigned long interval) {
  this->interval = interval;
  this->previousMillis = millis();
}

int Timer::Tick() {
  unsigned long currentMillis = millis();
  this->ShouldTrigger = currentMillis - this->previousMillis > this->interval;

  if (this->ShouldTrigger)
  {
    this->previousMillis = currentMillis;
  }
}
