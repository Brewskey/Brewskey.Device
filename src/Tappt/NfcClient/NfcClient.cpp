#include "NfcClient.h"

#define READ_TAG_TIME 70
#define EMULATE_TAG_TIME 370

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
  // this->swapTimer.Start();

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

  this->message.addLaunchApp("f523005d-37d3-4375-b3e8-4f1f56704f0f",
                             "d/" + data);
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

  return 0;
}

int NfcClient::Tick() {
#if DISABLE_NFC == 1
  return 0;
#endif

  switch (this->deviceNFCStatus) {
    case DeviceNFCStatus::DISABLED: {
      return 0;
    }

    case DeviceNFCStatus::PHONE_ONLY: {
      return this->SendMessage();
    }

    case DeviceNFCStatus::CARD_ONLY: {
      return this->ReadMessage();
    }

    // NOTE: This is really experimental and doesn't work well for Android :/
    case DeviceNFCStatus::PHONE_AND_CARD: {
      // this->swapTimer.Tick();
      // if (!this->swapTimer.ShouldTrigger()) {
      //   return NfcState::NO_MESSAGE;
      // }

      // pn532.setRFField(0x02, 0x01);
      // pn532.inRelease();

      this->state++;
      if (this->state == 3) {
        this->state = 0;
      }

      int result = 0;
      switch (this->state) {
        case 0: {
          result = this->ReadMessage();
          break;
        }

        default: {
          result = this->SendMessage();
          break;
        }
      }

      // pn532.setRFField(0x02, 0x00);
      return result;
    }

    default: {
      return 0;
    }
  }
}

NfcState::value NfcClient::ReadMessage() {
  // If reading authentication from a tag
  if (!this->nfcAdapter.tagPresent(READ_TAG_TIME)) {
    Serial.println("Tag not present");
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

NfcState::value NfcClient::SendMessage() {
  // Serial.println("Emulated Tag");
  if (nfc.emulate(EMULATE_TAG_TIME)) {
    return NfcState::SENT_MESSAGE;
  }

  return NfcState::NO_MESSAGE;
}

void NfcClient::SendPendingMessage() {
  if (this->readAuthenticationKey == "") {
    return;
  }

  this->serverLink->AuthorizePour(this->deviceId, this->readAuthenticationKey);
  this->readAuthenticationKey = "";
}
