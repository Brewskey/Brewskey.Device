#ifndef LED_h
#define LED_h

#include "application.h"
#include "ITick.h"

#ifndef RED_PIN
#define RED_PIN (D0)
#endif

#ifndef GREEN_PIN
#define GREEN_PIN (D1)
#endif

#ifndef BLUE_PIN
#define BLUE_PIN (D2)
#endif

#define PULSE_EPSILON 1

class LED : public ITick {
public:
  LED();
  void SetColor(int red, int green, int blue);
  void IsBreathing(bool isBreathing);
  virtual int Tick();
private:
  void Write(int red, int green, int blue);

  bool isBreathing = false;
  int red;
  int green;
  int blue;

  float brightness = 0;    // how bright the LED is
  float fadeAmount = .1;
};

#endif
