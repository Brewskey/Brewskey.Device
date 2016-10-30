#ifndef Pins_h
#define Pins_h

#include "application.h"

#define USE_INTERRUPT 1 // The flow sensor uses interrupt based measurement

#define RED_PIN (WKP)
#define GREEN_PIN (RX)
#define BLUE_PIN (TX)

// Configurations
// 0 - Photon - No NFC - OLED SPI Test
// 1 - Photon - Adafruit NFC
// 2 - Photon - Seeed NFC
// 3 - P1 Redboard - Seeed NFC
// 4 - PCB v1 - P1 + PN532 Chip
// 5 - PCB v2 - P1 + PN532 Chip
#define HARDWARE_CONFIG 0
#define SPI_HW_MODE

#if HARDWARE_CONFIG == 0
  #define OLED_SPI
  #define DISABLE_NFC 1
  #define SOLENOID_PIN (D2)
  #define FLOW_PIN (D6)
  #define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 1
   #define SPI_HW_MODE
  #define SS   (A2)
  #define SOLENOID_PIN (D2)
  #define FLOW_PIN (D6)
  #define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 2
  #define SS   (A2)
  #define SCK  (A3)
  #define MISO (A4)
  #define MOSI (A5)
  #define SOLENOID_PIN (D2)
  #define FLOW_PIN (D6)
  #define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 3
  #define SS   (A2)
  #define SCK  (A3)
  #define MISO (A4)
  #define MOSI (A5)
  #define SOLENOID_PIN (D2)
  #define FLOW_PIN (D6)
  #define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 4
  #define SPI_HW_MODE
  #define SS   DAC
  #define SOLENOID_PIN P1S1
  #define FLOW_PIN P1S0
  #define TEMPERATURE_PIN D2
#elif HARDWARE_CONFIG == 5
  #define SPI_HW_MODE
  #define SS   DAC
  #define SOLENOID_PIN P1S1
  #define FLOW_PIN P1S0
  #define TEMPERATURE_PIN D2
#endif

#ifdef OLED_SPI
  #define OLED_DC     D3
  #define OLED_CS     D4
  #define OLED_RESET  D5
#else
  #define OLED_RESET  4
#endif
#endif
