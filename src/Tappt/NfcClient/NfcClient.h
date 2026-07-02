#ifndef NfcClient_h
#define NfcClient_h

#include "NDEF/NdefMessage.h"
#include "NDEF/NfcAdapter.h"
#include "PN532/PN532.h"
#include "PN532/PN532_debug.h"
#include "PN532/emulatetag.h"
#include "PN532/snep.h"
#include "PN532_SPI/PN532_SPI.h"
#include "Tappt/ITick.h"
#include "Tappt/Pins.h"
#include "Tappt/ServerLink/DeviceNFCStatus.h"
#include "Tappt/ServerLink/ServerLink.h"

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
  virtual void Setup(ServerLink *serverLink);
  void SendPendingMessage();
  virtual int Tick();
  int Initialize(String data, uint8_t deviceNFCStatus);

 private:
  NfcState::value ReadMessage();
  // Drives phone-facing tag emulation; when withCardPolling is set it also
  // interleaves short reader bursts so physical cards keep working.
  NfcState::value PhoneTick(bool withCardPolling);
  // One reader-mode poll for a physical card, leaving the RF field off and
  // the chip idle afterwards so tag emulation can be re-armed.
  NfcState::value CardBurst();

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
  uint8_t uid[3] = {0x12, 0x34, 0x56};
  // How should the NFC be configured read/write/disabled. Disabled until
  // Initialize() provides the server-configured mode; PHONE_ONLY is 0, so
  // leaving this uninitialized armed emulation before the UID/NDEF were set.
  uint8_t deviceNFCStatus = DeviceNFCStatus::DISABLED;

  uint32_t lastCardPollTime = 0;
  uint32_t lastEmulatePollTime = 0;
  // Initialize() runs on the application thread while Tick() runs on the
  // NFC timer thread; chip commands are deferred here so only the timer
  // thread ever touches the SPI bus.
  volatile bool pendingChipSetup = false;
};

#endif
