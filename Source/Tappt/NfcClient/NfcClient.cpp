#include "NfcClient.h"

NfcClient::NfcClient(LED* led) :
#ifdef SPI_HW_MODE
  pn532spi(SPI, SS),
#else
  pn532spi(SCK, MISO, MOSI, SS),
#endif
  nfc(pn532spi), nfcAdapter(pn532spi)
{
  this->led = led;

  // This only needs to happen once for nfc & ndfAdapter
  nfc.init();
}

int NfcClient::Initialize(String data) {
  this->deviceId = String(data);
  Serial.print("Device ID: ");Serial.println(deviceId);

  this->message = NdefMessage();

  //this->message.addLaunchApp("A642F242-2F5B-4D0D-BB5A-10BFFAA9C33C", "d/" + deviceId);
  this->message.addUriRecord("tappt://"); // WP8.1 wasn't respecting launch app...
  this->message.addUriRecord("https://tappt.io/d/" + deviceId);
  this->message.addApplicationRecord("com.tappt.app");

  this->messageSize = this->message.getEncodedSize();
  if (this->messageSize > sizeof(this->ndefBuf)) {
      Serial.println("ndefBuf is too small");
      while (1) { }
  }

  message.encode(ndefBuf);

  // uid must be 3 bytes!
  nfc.setUid(uid);

  // comment out this command for no ndef message
  nfc.setNdefFile(ndefBuf, messageSize);

  return 0;
}

int NfcClient::Tick()
{
  Serial.println(this->deviceId);
  this->led->SetColor(0, 0, 255);
  NfcState::value output = this->SendMessage();
  if (output != NfcState::NO_MESSAGE) {
    return output;
  }

  this->led->SetColor(0, 20, 255);
  return this->ReadMessage();
}

NfcState::value NfcClient::ReadMessage()
{
  // If reading authentication from a tag
  if (!this->nfcAdapter.tagPresent(100))
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

  Serial.println(authenticationKey);
  Serial.println("printed");

  String deviceId = this->deviceId;

  sprintf(
    json,
    "{\"id\":\"%s\",\"tkn\":\"%s\"}",
    deviceId.c_str(),
    // remove \u0002 and "en"
    authenticationKey.substring(3).c_str()
  );

  Serial.print("Request Pour");Serial.println(json);
  Particle.publish("tappt_request-pour", json, 5, PRIVATE);

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
