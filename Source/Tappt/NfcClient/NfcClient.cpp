#include "NfcClient.h"

NfcClient::NfcClient() :
#ifdef SPI_HW_MODE
  pn532spi(SPI, SS),
#else
  pn532spi(SCK, MISO, MOSI, SS),
#endif
  pn532(pn532spi), nfc(pn532spi), nfcAdapter(pn532spi)
{
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

void NfcClient::Setup(ServerLink *serverLink) {
  this->serverLink = serverLink;
}


int NfcClient::Initialize(String data) {
#if DISABLE_NFC == 1
  return 0;
#endif
  this->deviceId = String(data);
  Serial.print("Device ID: ");Serial.println(deviceId);

  this->message = NdefMessage();

  this->message.addLaunchApp("f523005d-37d3-4375-b3e8-4f1f56704f0f", "d/" + deviceId);
  this->message.addUriRecord("https://brewskey.com/d/" + deviceId);
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

int NfcClient::Tick()
{
#if DISABLE_NFC == 1
  return 0;
#endif
  NfcState::value output;

  output = this->SendMessage();

  if (output != NfcState::NO_MESSAGE) {
    return output;
  }

  return output;

  return this->ReadMessage();
}

NfcState::value NfcClient::ReadMessage()
{
  // If reading authentication from a tag
  if (!this->nfcAdapter.tagPresent(70))
  {
    Serial.println("Tag not present");
    return NfcState::NO_MESSAGE;
  }

  NfcTag tag = this->nfcAdapter.read();

  if (!tag.hasNdefMessage())
  {
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

  this->serverLink->AuthorizePour(this->deviceId, authenticationKey);

  return NfcState::READ_MESSAGE;
}

NfcState::value NfcClient::SendMessage()
{
  Serial.println("Emulated Tag");
  if (nfc.emulate(300)) {
    return NfcState::SENT_MESSAGE;
  }

  return NfcState::NO_MESSAGE;
}
