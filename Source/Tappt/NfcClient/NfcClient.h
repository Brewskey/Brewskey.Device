#ifndef NfcClient_h
#define NfcClient_h

#include "PN532_SPI.h"
#include "PN532.h"
#include "snep.h"
#include "NdefMessage.h"
#include "NfcAdapter.h"

#include "ITick.h"
#include "RestClient.h"

namespace NfcState {
  enum value {
    ERROR = -1,
    NO_MESSAGE,
  };
};

class NfcClient {
public:
  NfcClient();
  virtual NfcState::value Tick();
private:
  NfcState::value ReadMessage();
  NfcState::value SendMessage();

  PN532_SPI pn532spi;
  NfcAdapter nfc;
  //SNEP snep;

  uint8_t ndefBuf[128];
};

#endif
