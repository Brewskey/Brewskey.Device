
#include "PN532_SPI.h"
#include "PN532_debug.h"
#include "application.h"

#define STATUS_READ     2
#define DATA_WRITE      1
#define DATA_READ       3

#ifdef SPI_HW_MODE
PN532_SPI::PN532_SPI(SPIClass &spi, uint8_t ss)
{
    command = 0;
    _spi = &spi;
    _ss  = ss;
}
#else
PN532_SPI::PN532_SPI(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t ss) {
  _clk = clk;
  _miso = miso;
  _mosi = mosi;
  _ss = ss;
}
#endif

void PN532_SPI::begin()
{
#ifdef SPI_HW_MODE
    pinMode(_ss, OUTPUT);

    _spi->setClockDivider(SPI_CLOCK_DIV16); // set clock 2MHz(max: 5MHz)
    _spi->setDataMode(SPI_MODE0);  // PN532 only supports mode0
    _spi->setBitOrder(LSBFIRST);
    _spi->begin(_ss);
#else
    // Setup Software SPI
    pinMode(_ss, OUTPUT);
    pinMode(_clk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);

    #ifdef PN532DEBUG
      Serial.print("[SPI PINS] CLK:"); Serial.print(_clk);
      Serial.print(",MISO:"); Serial.print(_miso);
      Serial.print(",MOSI:"); Serial.print(_mosi);
      Serial.print(",SS:"); Serial.println(_ss);
    #endif
#endif
}

void PN532_SPI::wakeup()
{
    digitalWrite(_ss, LOW);
    delay(1000);
    digitalWrite(_ss, HIGH);
}



int8_t PN532_SPI::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    uint16_t timer = 0;
    writeFrame(header, hlen, body, blen);

    while (!isReady()) {
        timer+=10;
        if (timer > PN532_ACK_WAIT_TIME){
            DMSG("Time out when waiting for ACK\n");
            return -2;
        }
        delay(10);
    }

    if (readAckFrame()) {
        DMSG("Invalid ACK\n");
        return PN532_INVALID_ACK;
    }
    return 0;
}

int16_t PN532_SPI::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint16_t time = 0;
    while (!isReady()) {
        delay(1);
        time++;
        if (timeout > 0 && time > timeout) {
            return PN532_TIMEOUT;
        }
    }

    digitalWrite(_ss, LOW);
    delay(2);

    int16_t result;
    do {
        write(DATA_READ);

        if (0x00 != read()      ||       // PREAMBLE
                0x00 != read()  ||       // STARTCODE1
                0xFF != read()           // STARTCODE2
           ) {
            Serial.println("invalid 1");
            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t length = read();
        if (0 != (uint8_t)(length + read())) {   // checksum of length
            Serial.println("invalid 2");
            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t cmd = command + 1;               // response command
        if (PN532_PN532TOHOST != read() || (cmd) != read()) {
            Serial.println("invalid 3");
            result = PN532_INVALID_FRAME;
            break;
        }

        DMSG("read:  ");
        DMSG_HEX(cmd);

        length -= 2;
        if (length > len) {
            for (uint8_t i = 0; i < length; i++) {
                DMSG_HEX(read());                 // dump message
            }
            DMSG("\nNot enough space\n");
            read();
            read();
            result = PN532_NO_SPACE;  // not enough space
            break;
        }

        uint8_t sum = PN532_PN532TOHOST + cmd;
        for (uint8_t i = 0; i < length; i++) {
            buf[i] = read();
            sum += buf[i];

            DMSG_HEX(buf[i]);
        }
        DMSG('\n');

        uint8_t checksum = read();
        if (0 != (uint8_t)(sum + checksum)) {
            DMSG("checksum is not ok\n");
            result = PN532_INVALID_FRAME;
            break;
        }
        read();         // POSTAMBLE

        result = length;
    } while (0);

    digitalWrite(_ss, HIGH);

    return result;
}

boolean PN532_SPI::isReady()
{
    digitalWrite(_ss, LOW);
    delay(2);
    write(STATUS_READ);
    uint8_t status = read() & 1;
    digitalWrite(_ss, HIGH);

    return status;
}

void PN532_SPI::writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen) {
    digitalWrite(_ss, LOW);
    delay(2);               // wake up PN532

    write(DATA_WRITE);
    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);

    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    write(length);
    write(~length + 1);         // checksum of length

    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    DMSG("write: ");

    for (uint8_t i = 0; i < hlen; i++) {
        write(header[i]);
        sum += header[i];

        DMSG_HEX(header[i]);
    }
    for (uint8_t i = 0; i < blen; i++) {
        write(body[i]);
        sum += body[i];

        DMSG_HEX(body[i]);
    }

    uint8_t checksum = ~sum + 1;        // checksum of TFI + DATA
    write(checksum);
    write(PN532_POSTAMBLE);

    digitalWrite(_ss, HIGH);

    DMSG('\n');
}

int8_t PN532_SPI::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    uint8_t ackBuf[sizeof(PN532_ACK)];

    digitalWrite(_ss, LOW);
    delay(2);
    write(DATA_READ);

    for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) {
        delay(4);
        ackBuf[i] = read();
    }

    digitalWrite(_ss, HIGH);

    int output = memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
    DMSG("Ack Output: ");
    for(int i = 0; i < sizeof(PN532_ACK); i++)
    {
        DMSG_HEX(ackBuf[i]);
    }

    return output;
}

void PN532_SPI::write(uint8_t data) {
#ifdef SPI_HW_MODE
    _spi->transfer(data);
#else
    PIN_MAP[_mosi].gpio_peripheral->BRR = PIN_MAP[_mosi].gpio_pin; // Start with Data Low (MODE0)
    for (uint8_t bit=0; bit<8; bit++) {
      asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
      PIN_MAP[_clk].gpio_peripheral->BRR = PIN_MAP[_clk].gpio_pin; // Clock Low
      if (data & (1<<bit)) { // walks up mask from bit 0 to bit 7
        PIN_MAP[_mosi].gpio_peripheral->BSRR = PIN_MAP[_mosi].gpio_pin; // Data High
      } else {
        PIN_MAP[_mosi].gpio_peripheral->BRR = PIN_MAP[_mosi].gpio_pin; // Data Low
      }
      asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
      PIN_MAP[_clk].gpio_peripheral->BSRR = PIN_MAP[_clk].gpio_pin; // Clock High (Data Shifted Out)
    }
    asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
    PIN_MAP[_clk].gpio_peripheral->BRR = PIN_MAP[_clk].gpio_pin; // Return Clock Low (MODE0)
    PIN_MAP[_mosi].gpio_peripheral->BSRR = PIN_MAP[_mosi].gpio_pin; // Return Data High (MODE0)
#endif
};

uint8_t PN532_SPI::read() {
#ifdef SPI_HW_MODE
    return _spi->transfer(0);
#else
    uint8_t x = 0;
    for (uint8_t bit=0; bit<8; bit++)  {
      PIN_MAP[_clk].gpio_peripheral->BSRR = PIN_MAP[_clk].gpio_pin; // Clock High (Data Shifted In)

      if (PIN_MAP[_miso].gpio_peripheral->IDR & PIN_MAP[_miso].gpio_pin) {
        x |= (1<<bit);
      }
      //asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
      PIN_MAP[_clk].gpio_peripheral->BRR = PIN_MAP[_clk].gpio_pin; // Clock Low (On exit, Clock Low (MODE0))
    }
    
    return x;
#endif
};
