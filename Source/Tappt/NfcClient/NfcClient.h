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
#include "Timer.h"

namespace NfcState {
  enum value {
    ERROR = -1,
    NO_MESSAGE,
    SENT_MESSAGE,
    READ_MESSAGE,
  };
};

class NfcClient: public ITick {
public:
  NfcClient();
  virtual int Tick();
private:
  int Initialize(String id);
  NfcState::value ReadMessage();
  NfcState::value SendMessage();

  String deviceId;
  Timer getIdTimer = Timer(10000);

  PN532_SPI pn532spi;

  EmulateTag nfc;
  NfcAdapter nfcAdapter;

  NdefMessage message;
  int messageSize;
  uint8_t ndefBuf[128];
  uint8_t uid[3] = { 0x12, 0x34, 0x56 };
};

#endif
