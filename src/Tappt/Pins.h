#ifndef Pins_h
#define Pins_h

#include "application.h"

// #define TEST_MODE 1
#define USE_INTERRUPT 1  // The flow sensor uses interrupt based measurement

#define RED_PIN (WKP)
#define GREEN_PIN (RX)
#define BLUE_PIN (TX)

// Configurations
// 0 - Photon - No NFC - OLED SPI Test
// 1 - Photon - Adafruit NFC
// 2 - Photon - Seeed NFC
// 3 - P1 Redboard - Seeed NFC
// 4 - PCB v1/v2 - P1 + PN532 Chip (no screen + I2C screen)
// 5 - PCB v3 - P1 + PN532 Chip
// 6 - PCB v3.1 - P1 + PN532 Chip + BIG SCREEN
#define HARDWARE_CONFIG 6
// #define SHOW_OUTPUT 2

#define MAX_TAP_COUNT_PER_BOX 4

#define BREWSKEY_PRODUCT_VERSION 17

#if HARDWARE_CONFIG == 0
#define BREWSKEY_PRODUCT_ID 2
#define OLED_SPI
//  #define SPI_HW_MODE
#define BIG_SCREEN
#define DISABLE_NFC 1
#define SOLENOID_PIN (D2)
#define FLOW_PIN (D6)
#define TEMPERATURE_PIN (D7)
#elif HARDWARE_CONFIG == 1
#define BREWSKEY_PRODUCT_ID 2
#define SPI_HW_MODE
#define SS (A2)
#define SOLENOID_PIN (D2)
#define FLOW_PIN (D6)
#define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 2
#define BREWSKEY_PRODUCT_ID 3
#define SS (A2)
#define SCK (A5)
#define MISO (A4)
#define MOSI (A5)
#define SOLENOID_PIN (D2)
#define FLOW_PIN (D6)
#define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 3
#define BREWSKEY_PRODUCT_ID 4
#define SS (A2)
#define SCK (A3)
#define MISO (A4)
#define MOSI (A5)
#define SOLENOID_PIN (D2)
#define FLOW_PIN (D6)
#define TEMPERATURE_PIN (D4)
#elif HARDWARE_CONFIG == 4
#define BREWSKEY_PRODUCT_ID 5
#define SPI_HW_MODE
#define SS DAC
#define SOLENOID_PIN P1S1
#define FLOW_PIN P1S0
#define TEMPERATURE_PIN D2
#elif HARDWARE_CONFIG == 5
#define BREWSKEY_PRODUCT_ID 1
// #define DISABLE_NFC 1
#define EXPANSION_BOX_PIN A0
//  #define SPI_HW_MODE
#define OLED_SPI
#define SS DAC
#define SOLENOID_PIN P1S1
#define FLOW_PIN P1S0
#define TEMPERATURE_PIN D2
#define OLED_DC P1S3
#define OLED_CS A2
#define OLED_RESET A1
#define USE_BETA_PACKET_FORMAT 1
#elif HARDWARE_CONFIG == 6
#define BREWSKEY_PRODUCT_ID 2
#define BIG_SCREEN
#define EXPANSION_BOX_PIN A0
#define OLED_SPI
#define SS DAC
#define SOLENOID_PIN P1S1
#define FLOW_PIN P1S0
#define TEMPERATURE_PIN D2
#define OLED_DC P1S3
#define OLED_CS A2
#define OLED_RESET A1
#endif

#ifndef OLED_RESET
#define OLED_DC D3
#define OLED_CS D4
#define OLED_RESET D5
#endif

#endif
