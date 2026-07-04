#include "NfcClient.h"

// Reader-burst card poll window (ms). With MxRtyPassiveActivation = 1 the
// chip answers "no target" well inside this window; it is only a host-side
// safety timeout.
#define READ_TAG_TIME 70
// How often to check a pending target arm for activation (ms).
#define EMULATE_POLL_INTERVAL 15
// How often PHONE_AND_CARD interrupts tag emulation to poll for a physical
// card (ms). Emulation stays armed ~90% of the time.
#define CARD_POLL_INTERVAL 500
// Card poll cadence when there is no emulation to interleave with (ms).
#define CARD_ONLY_POLL_INTERVAL 100

// Target activation bytes (MODE, SENS_RES byte 0, SENS_RES byte 1, SEL_RES)
// presented to phones during tag emulation. Variant 1 (unrestricted MODE
// 0x00, ATQA 0x0004, SAK 0x20) is the field-validated config: PICC-only
// MODE 0x05 (variant 0, the historical config) makes Android detect
// nothing. Non-ISO-DEP activations are detected and released in
// EmulateTag::emulatePoll. These bytes do not affect card reading.
#define NFC_TARGET_VARIANT 1
static const uint8_t TARGET_VARIANTS[][4] = {
    {0x05, 0x04, 0x00, 0x20},
    {0x00, 0x04, 0x00, 0x20},
};

NfcClient::NfcClient()
    :
#ifdef SPI_HW_MODE
      pn532spi(SPI, SS),
#else
      pn532spi(SCK, MISO, MOSI, SS),
#endif
      pn532(pn532spi),
      nfc(pn532spi),
      nfcAdapter(pn532spi) {
#if DISABLE_NFC == 1
  return;
#endif

  // This only needs to happen once for nfc & ndfAdapter
  if (!nfc.init()) {
    DMSG("Error initializing PN532\r\n");
    RGB.control(true);
    RGB.color(255, 128, 0);

    while (1) {
      Particle.process();
      delay(10);
    }
  }
}

void NfcClient::Setup(ServerLink* serverLink) { this->serverLink = serverLink; }

int NfcClient::Initialize(String data, uint8_t deviceNFCStatus) {
#if DISABLE_NFC == 1
  return 0;
#endif
  this->deviceId = String(data).toInt();
  this->deviceNFCStatus = deviceNFCStatus;
  Serial.print("Device ID: ");
  Serial.println(deviceId);

  this->message = NdefMessage();

  // Record order matters: iOS background reading uses the FIRST URI record,
  // so the URI leads. The Android Application Record then makes Android
  // route the tag straight to the app - or to its Play Store listing when
  // the app is not installed - instead of opening the URL in a browser.
  this->message.addUriRecord("https://brewskey.com/d/" + data);
  this->message.addApplicationRecord("com.brewskey.app");

  this->messageSize = this->message.getEncodedSize();
  if (this->messageSize > sizeof(this->ndefBuf)) {
    Serial.println("ndefBuf is too small");
    return -1;
  }

  message.encode(ndefBuf);

  // uid must be 3 bytes!
  nfc.setUid(uid);

  // comment out this command for no ndef message
  nfc.setNdefFile(ndefBuf, messageSize);

  nfc.setTagWriteable(false);

  // Memory-only; takes effect on the next arm.
  nfc.setTargetParams(TARGET_VARIANTS[NFC_TARGET_VARIANT][0],
                      TARGET_VARIANTS[NFC_TARGET_VARIANT][1],
                      TARGET_VARIANTS[NFC_TARGET_VARIANT][2],
                      TARGET_VARIANTS[NFC_TARGET_VARIANT][3]);

  // Chip reconfiguration (abort any pending arm + retry limit) happens on
  // the NFC timer thread; see Tick(). This method runs on the application
  // thread and must not touch the SPI bus.
  this->pendingChipSetup = true;

  return 0;
}

int NfcClient::Tick() {
#if DISABLE_NFC == 1
  return 0;
#endif

  if (this->pendingChipSetup) {
    this->pendingChipSetup = false;
    this->nfc.emulateAbort();
    // Bounded card polls: without this, a card poll that finds no target
    // keeps InListPassiveTarget pending inside the chip with the RF field
    // up, which fights any phone that is simultaneously trying to read us.
    // 0x01 = one retry (up to two activation attempts) per poll.
    this->pn532.setPassiveActivationRetries(0x01);
  }

  switch (this->deviceNFCStatus) {
    case DeviceNFCStatus::DISABLED: {
      return 0;
    }

    case DeviceNFCStatus::PHONE_ONLY: {
      return this->PhoneTick(false);
    }

    case DeviceNFCStatus::CARD_ONLY: {
      if ((millis() - this->lastCardPollTime) < CARD_ONLY_POLL_INTERVAL) {
        return NfcState::NO_MESSAGE;
      }
      NfcState::value result = this->CardBurst();
      this->lastCardPollTime = millis();
      return result;
    }

    case DeviceNFCStatus::PHONE_AND_CARD: {
      return this->PhoneTick(true);
    }

    default: {
      return 0;
    }
  }
}

NfcState::value NfcClient::PhoneTick(bool withCardPolling) {
  uint32_t now = millis();

  if (withCardPolling &&
      (now - this->lastCardPollTime) >= CARD_POLL_INTERVAL) {
    // Briefly interrupt emulation to look for a physical card. Emulation is
    // passive (no RF field of its own), so this reader burst is the only
    // window where we can collide with a phone; keep it short.
    int8_t abortStatus = this->nfc.emulateAbort();
    if (abortStatus == EMULATE_POLL_MESSAGE_READ) {
      // A phone activated us just before the burst; the transaction was
      // served instead of aborted. Skip this burst cycle.
      this->lastCardPollTime = millis();
      return NfcState::SENT_MESSAGE;
    }

    NfcState::value result = this->CardBurst();
    this->lastCardPollTime = millis();
    return result;
  }

  if (!this->nfc.isArmed()) {
    // Park the chip as a passive target. It stays armed indefinitely (and
    // costs nothing) until a phone activates it or a card burst aborts it.
    this->nfc.emulateStart();
    return NfcState::NO_MESSAGE;
  }

  if ((now - this->lastEmulatePollTime) < EMULATE_POLL_INTERVAL) {
    return NfcState::NO_MESSAGE;
  }
  this->lastEmulatePollTime = now;

  int8_t status = this->nfc.emulatePoll();
  if (status == EMULATE_POLL_MESSAGE_READ) {
    return NfcState::SENT_MESSAGE;
  }

  // EMULATE_POLL_FAILED leaves the chip disarmed; it is re-armed on the
  // next tick.
  return NfcState::NO_MESSAGE;
}

NfcState::value NfcClient::CardBurst() {
  NfcState::value result = this->ReadMessage();

  // If the poll timed out host-side the InListPassiveTarget command may
  // still be pending inside the chip; abort it before issuing new commands.
  // (An ACK frame when no command is pending is ignored.)
  this->pn532.abortCommand();

  // Kill the reader RF field so it cannot interfere with phones trying to
  // activate us as a target.
  this->pn532.setRFField(0x00, 0x00);

  return result;
}

NfcState::value NfcClient::ReadMessage() {
  // If reading authentication from a tag
  if (!this->nfcAdapter.tagPresent(READ_TAG_TIME)) {
    return NfcState::NO_MESSAGE;
  }

  NfcTag tag = this->nfcAdapter.read();

  if (!tag.hasNdefMessage()) {
    Serial.println("No message");
    return NfcState::NO_MESSAGE;
  }

  int totalLength = 0;
  NdefMessage message = tag.getNdefMessage();

  int recordCount = message.getRecordCount();
  String authenticationKey;

  for (int i = 0; i < recordCount; i++) {
    NdefRecord record = message[i];
    int length = record.getPayloadLength();

    totalLength += length;

    byte* payload = new byte[length + 1];
    payload[length] = (byte)'\0';
    record.getPayload(payload);

    authenticationKey += String((const char*)payload);
    delete[] payload;
  }

  if (authenticationKey.length() == 0) {
    return NfcState::NO_MESSAGE;
  }

  this->readAuthenticationKey = authenticationKey;

  return NfcState::READ_MESSAGE;
}

void NfcClient::SendPendingMessage() {
  if (this->readAuthenticationKey == "") {
    return;
  }

  this->serverLink->AuthorizePour(this->deviceId, this->readAuthenticationKey);
  this->readAuthenticationKey = "";
}
