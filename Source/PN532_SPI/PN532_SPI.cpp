
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
    _spi->setClockDivider(SPI_CLOCK_DIV16); // Photon runs at 120Mhz so 3.75Mhz
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
    delay(500);
    digitalWrite(_ss, HIGH);
}



int8_t PN532_SPI::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    command = header[0];
    uint16_t timer = 0;
    writeFrame(header, hlen, body, blen);

    while (!isReady()) {
        timer+=1;
        if (timer > PN532_ACK_WAIT_TIME){
            DMSG("Time out when waiting for ACK\r\n");
            return -2;
        }
        delay(1);
    }

    if (readAckFrame()) {
        DMSG("Invalid ACK\r\n");
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
    //delay(2);

    int16_t result;
    do {
        write(DATA_READ);
        uint8_t tmp[3];

        /** Frame Preamble and Start Code */
        if(receive(tmp, 3, timeout)<=0){
            DMSG("\r\nFrame Preamble and Start Code timeout\r\n");
            return PN532_TIMEOUT;
        }
        if(0 != tmp[0] || 0!= tmp[1] || 0xFF != tmp[2]){
            DMSG("\r\nPreamble error");
            return PN532_INVALID_FRAME;
        }

        /** receive length and check */
        uint8_t length[2];
        if(receive(length, 2, timeout) <= 0){
            DMSG("\r\nreceive length and check timeout\r\n");
           return PN532_TIMEOUT;
        }
        if( 0 != (uint8_t)(length[0] + length[1]) ){
           DMSG("\r\nLength error");
           return PN532_INVALID_FRAME;
        }
        length[0] -= 2;
        if( length[0] > len){
          DMSG("\r\nNo Space error\r\n");
          return PN532_NO_SPACE;
        }

        /** receive command byte */
        uint8_t cmd = command + 1;               // response command
        if(receive(tmp, 2, timeout) <= 0){
            DMSG("\r\nreceive command timeout\r\n");
            return PN532_TIMEOUT;
        }
        if( PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]){
            DMSG("\r\nCommand error");
            return PN532_INVALID_FRAME;
        }

        DMSG(">");
        DMSG_HEX(cmd);

        if(receive(buf, length[0], timeout) != length[0]){
            DMSG("\r\nTimeout???");
            return PN532_TIMEOUT;
        }

        uint8_t sum = PN532_PN532TOHOST + cmd;
        for(uint8_t i=0; i<length[0]; i++){
            sum += buf[i];
            DMSG_HEX(buf[i]);
        }

        DMSG("\r\n");

        /** checksum and postamble */
        if(receive(tmp, 2, timeout) <= 0){
            DMSG("\r\nChecksum timeout\r\n");
            return PN532_TIMEOUT;
        }
        if( 0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1] ){
            DMSG("\r\nChecksum error");
            return PN532_INVALID_FRAME;
        }

        result = length[0];
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
    //delay(1);               // wake up PN532

    write(DATA_WRITE);
    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);

    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    write(length);
    write(~length + 1);         // checksum of length

    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    DMSG("<");

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

    DMSG("\r\n");
}

int8_t PN532_SPI::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    uint8_t ackBuf[sizeof(PN532_ACK)];

    digitalWrite(_ss, LOW);
    //delay(1);
    write(DATA_READ);

    if( receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0 ){
        DMSG("Timeout\n");
        return PN532_TIMEOUT;
    }

    digitalWrite(_ss, HIGH);

    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
}

int8_t PN532_SPI::receive(uint8_t *buf, int len, uint16_t timeout)
{
  int read_bytes = 0;
  int ret;
  unsigned long start_millis;

  while (read_bytes < len) {
    start_millis = millis();
    do {
      ret = read();
      if (ret >= 0) {
        break;
     }
     delay(1);
    } while((timeout == 0) || ((millis()- start_millis ) < timeout));

    if (ret < 0) {
        if(read_bytes){
            return read_bytes;
        }else{
            return PN532_TIMEOUT;
        }
    }
    buf[read_bytes] = (uint8_t)ret;
    read_bytes++;
  }
  return read_bytes;
}

void PN532_SPI::write(uint8_t data) {
  #ifdef SPI_HW_MODE
    SPI.transfer(data);
  #else
  #ifdef SPARK_CORE
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
  #else
    digitalWrite(_mosi, HIGH);

    for (uint8_t i=0; i<8; i++) {
      digitalWrite(_clk, LOW);
      if (data & (1<<i)) {
        digitalWrite(_mosi, HIGH);
      } else {
        digitalWrite(_mosi, LOW);
      }
      digitalWrite(_clk, HIGH);
    }
    digitalWrite(_clk, LOW);
    digitalWrite(_mosi, HIGH);
  #endif // Spark
  #endif // HW SPI
};

uint8_t PN532_SPI::read() {
  #ifdef SPI_HW_MODE
    uint8_t x = SPI.transfer(0x55);
  #else
  #ifdef SPARK_CORE
    uint8_t x = 0;

    for (uint8_t bit=0; bit<8; bit++)  {
      PIN_MAP[_clk].gpio_peripheral->BSRR = PIN_MAP[_clk].gpio_pin; // Clock High (Data Shifted In)

      if (PIN_MAP[_miso].gpio_peripheral->IDR & PIN_MAP[_miso].gpio_pin) {
        x |= (1<<bit);
      }
      //asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
      PIN_MAP[_clk].gpio_peripheral->BRR = PIN_MAP[_clk].gpio_pin; // Clock Low (On exit, Clock Low (MODE0))
    }
  #else
    uint8_t x = 0;

    for (uint8_t i=0; i<8; i++) {
      digitalWrite(_clk, HIGH);
      if (digitalRead(_miso)) {
        x |= (1<<i);
      }
      digitalWrite(_clk, LOW);
    }
  #endif //Spark
  #endif //HW SPI
    return x;
};
