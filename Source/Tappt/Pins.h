#ifndef Pins_h
#define Pins_h

#include "application.h"

#define IS_PROTOTYPE 0
#define SPI_HW_MODE
#define USE_FAKE_POUR 0

#define RED_PIN (D0)
#define GREEN_PIN (D1)
#define BLUE_PIN (D2)

#define TEMPERATURE_PIN (D4)
#define SOLENOID_PIN (D5)
#define FLOW_PIN (D6)

#if IS_PROTOTYPE == 0
#define SS   (A2)
#define SCK  (A3)
#define MISO (A4)
#define MOSI (A5)
#else
//#define SCK  (A3)
//#define MOSI (A5)
#define SS   DAC
//#define MISO (A4)
#endif

#endif
