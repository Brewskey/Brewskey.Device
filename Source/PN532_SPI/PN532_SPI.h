
#ifndef __PN532_SPI_H__
#define __PN532_SPI_H__

#include "application.h"
#include "PN532Interface.h"

#if PLATFORM_ID == 6
//#define SPI_HW_MODE 1
#endif

class PN532_SPI : public PN532Interface {
public:
#ifdef SPI_HW_MODE
    PN532_SPI(SPIClass &spi, uint8_t ss);
#else
    PN532_SPI(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t ss);
#endif

    void begin();
    void wakeup();
    int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);

private:
    SPIClass* _spi;

    uint8_t   _clk;
    uint8_t   _miso;
    uint8_t   _mosi;
    uint8_t   _ss;

    uint8_t command;

    boolean isReady();
    void writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int8_t readAckFrame();

    void write(uint8_t data);
    uint8_t read();

    /**************Conditional fast pin access for Core and Photon*****************/
    #if PLATFORM_ID == 0 // Core
      inline void digitalWriteFastLow(int pin) {
        PIN_MAP[pin].gpio_peripheral->BRR = PIN_MAP[pin].gpio_pin;
      }

      inline void digitalWriteFastHigh(int pin) {
        PIN_MAP[pin].gpio_peripheral->BSRR = PIN_MAP[pin].gpio_pin;
      }

    #elif PLATFORM_ID == 6 // Photon
      STM32_Pin_Info* PIN_MAP = HAL_Pin_Map(); // Pointer required for highest access speed

      inline void digitalWriteFastLow(int pin) {
        PIN_MAP[pin].gpio_peripheral->BSRRH = PIN_MAP[pin].gpio_pin;
      }

      inline void digitalWriteFastHigh(int pin) {
        PIN_MAP[pin].gpio_peripheral->BSRRL = PIN_MAP[pin].gpio_pin;
      }

    #else
      #error "*** PLATFORM_ID not supported by this library. PLATFORM should be Core or Photon ***"
    #endif
};

#endif
