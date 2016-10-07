#ifndef Pins_h
#define Pins_h

#include "application.h"

#define IS_PROTOTYPE 1
//#define DISABLE_NFC 1
#define SPI_HW_MODE
#define USE_FAKE_POUR 0
#define USE_INTERRUPT 1

#define RED_PIN (WKP)
#define GREEN_PIN (RX)
#define BLUE_PIN (TX)

#if IS_PROTOTYPE == 0

#define SS   (A2)
#define SCK  (A3)
#define MISO (A4)
#define MOSI (A5)

#define SOLENOID_PIN (D2)
#define FLOW_PIN (D6)
#define TEMPERATURE_PIN (D4)

#else

#define SS   DAC
#define SOLENOID_PIN P1S1
#define FLOW_PIN P1S0
#define TEMPERATURE_PIN D2

#endif

#endif
