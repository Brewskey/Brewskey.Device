#include "Solenoid.h"

Solenoid::Solenoid()
{
  pinMode(SOLENOID_PIN, OUTPUT);

  // turn on
  digitalWrite(SOLENOID_PIN, HIGH);
  // turn off
  digitalWrite(SOLENOID_PIN, LOW);
}

void Solenoid::Pour() {

}
