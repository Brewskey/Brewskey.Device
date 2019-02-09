#ifndef NfcClient_h
#define NfcClient_h

#include "PN532_SPI/PN532_SPI.h"
#include "PN532/PN532.h"
#include "PN532/PN532_debug.h"
#include "PN532/emulatetag.h"
#include "PN532/snep.h"
#include "NDEF/NdefMessage.h"
#include "NDEF/NfcAdapter.h"

#include "Tappt/ITick.h"
#include "Tappt/led/LED.h"
#include "Tappt/Pins.h"
#include "Tappt/ServerLink/ServerLink.h"
#include "Tappt/TapptTimer/TapptTimer.h"
//#define P2P 1

namespace NfcState {
  enum value {
    NFC_ERROR = -1,
    NO_MESSAGE,
    SENT_MESSAGE,
    READ_MESSAGE,
  };
};

class NfcClient : public ITick {
public:
  NfcClient();
  void Setup(ServerLink *serverLink);
  void SendPendingMessage();
  virtual int Tick();
  int Initialize(String data);
private:
  NfcState::value ReadMessage();
  NfcState::value SendMessage();

  uint32_t deviceId;
  String readAuthenticationKey = "";

  PN532_SPI pn532spi;
  PN532 pn532;
  ServerLink *serverLink;
  EmulateTag nfc;
  NfcAdapter nfcAdapter;

  NdefMessage message;
  int messageSize;
  uint8_t ndefBuf[256];
  uint8_t uid[3] = { 0x12, 0x34, 0x56 };
  TapptTimer swapTimer = TapptTimer(500);
  // Either read mode or write mode
  uint8_t state = 0;
};

#endif
