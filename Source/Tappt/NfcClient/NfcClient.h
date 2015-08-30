#ifndef NfcClient_h
#define NfcClient_h

#include "PN532_SPI.h"
#include "PN532.h"
#include "emulatetag.h"
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

  EmulateTag nfc;
  //SNEP nfc;
  //NfcAdapter nfc;

  int messageSize;
  NdefMessage message;
  uint8_t ndefBuf[128];
  uint8_t uid[3] = { 0x12, 0x34, 0x56 };
};

#endif
