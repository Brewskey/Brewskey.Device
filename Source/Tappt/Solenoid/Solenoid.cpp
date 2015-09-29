#include "Solenoid.h"

Solenoid::Solenoid()
{
  pinMode(SOLENOID_PIN, OUTPUT);

  this->Open();
  delay(10);
  this->Close();
}

void Solenoid::Open()
{
  digitalWrite(SOLENOID_PIN, HIGH);
}

void Solenoid::Close()
{
  digitalWrite(SOLENOID_PIN, LOW);
}
