#ifndef LED_h
#define LED_h

#include "application.h"
#include "Pins.h"

#define PULSE_EPSILON 1

class LED {
public:
  LED();
  void handler(uint8_t r, uint8_t g, uint8_t b);
};

#endif
