#ifndef LED_h
#define LED_h

#include "application.h"

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

class LED {
public:
  LED();
  void handler(uint8_t r, uint8_t g, uint8_t b);
};

#endif
