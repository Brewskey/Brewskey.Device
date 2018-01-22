#ifndef LED_h
#define LED_h

#include "application.h"
#include "Tappt/Pins.h"

class LED {
public:
  LED();
  void handler(uint8_t r, uint8_t g, uint8_t b);
};

#endif
