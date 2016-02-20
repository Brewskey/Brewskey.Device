
#ifndef __PN532_SPI_H__
#define __PN532_SPI_H__

#include "application.h"
#include "Pins.h"
#include "PN532Interface.h"

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
    int8_t receive(uint8_t *buf, int len, uint16_t timeout);
    void writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int8_t readAckFrame();

    void write(uint8_t data);
    uint8_t read();
};

#endif
