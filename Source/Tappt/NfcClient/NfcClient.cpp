#include "NfcClient.h"

void test(const char *eventName, const char *data) {
  Serial.println("FOOOOOO");
}

NfcClient::NfcClient(LED* led) :
#ifdef SPI_HW_MODE
  pn532spi(SPI, SS),
#else
  pn532spi(SCK, MISO, MOSI, SS),
#endif
  nfc(pn532spi), nfcAdapter(pn532spi)
{
  this->led = led;

  Particle.function("initialize", &NfcClient::Initialize, this);
  Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);

  // This only needs to happen once for nfc & ndfAdapter
  nfc.init();
}

int NfcClient::Initialize(String data) {
  this->deviceId = String(data);
  Serial.print("Device ID: ");Serial.println(deviceId);

  this->message = NdefMessage();

  this->message.addUriRecord("tappt://view-tap?deviceId=" + deviceId);

  this->messageSize = this->message.getEncodedSize();
  if (this->messageSize > sizeof(this->ndefBuf)) {
      Serial.println("ndefBuf is too small");
      while (1) { }
  }

  message.encode(ndefBuf);

  return 0;
}

int NfcClient::Tick()
{
  if (this->deviceId == NULL || this->deviceId == "") {
    this->getIdTimer.Tick();

    if (this->getIdTimer.ShouldTrigger) {
      Serial.println("Requesting DeviceId");
      Particle.publish("tappt_initialize", (const char *)0, 5, PRIVATE);
    }

    return NfcState::NO_MESSAGE;
  }

  this->led->IsBreathing(true);
  this->led->SetColor(0, 0, 255);
  NfcState::value output = this->SendMessage();
  if (output != NfcState::NO_MESSAGE) {
    return output;
  }

  this->led->SetColor(0, 255, 255);
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

  return NfcState::NO_MESSAGE;
}

NfcState::value NfcClient::SendMessage()
{
  // comment out this command for no ndef message
  nfc.setNdefFile(ndefBuf, messageSize);

  // uid must be 3 bytes!
  nfc.setUid(uid);
  nfc.emulate(1200);
  Serial.println("Emulated Tag");

  return NfcState::NO_MESSAGE;
}
