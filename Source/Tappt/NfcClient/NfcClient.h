#ifndef NfcClient_h
#define NfcClient_h

#include "PN532_SPI.h"
#include "PN532.h"
#include "snep.h"
#include "NdefMessage.h"
#include "NfcAdapter.h"

#include "ITick.h"

namespace NfcState {
  enum e {
    NO_MESSAGE,
  };
};

class NfcClient {
public:
  NfcClient();
  virtual int Tick();
private:
  int ReadMessage();
  int SendMessage();

  PN532_SPI pn532spi;
  NfcAdapter nfc;
  //SNEP snep;

  uint8_t ndefBuf[128];
};

#endif
