#include "LED.h"

LED::LED() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void LED::SetColor(int red, int green, int blue) {
  this->red = red;
  this->green = green;
  this->blue = blue;

  this->Write(this->red, this->green, this->blue);
}
void LED::IsBreathing(bool isBreathing) {
  this->isBreathing = isBreathing;
}

int LED::Tick() {
  if (this->isBreathing == false) {
    return 0;
  }

  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount ;
  }

  float percentage = brightness / 255.0;
  this->Write(
    this->red * percentage,
    this->green * percentage,
    this->blue * percentage
  );

  return 0;
}

void LED::Write(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}
