#include "NfcClient.h"

NfcClient::NfcClient() :
  pn532spi(SCK, MISO, MOSI, SS), nfc(pn532spi)//, snep(pn532spi)
{
  this->nfc.begin();
}

int NfcClient::Tick()
{
  /*
  // Read message over peer-to-peer
  Serial.println("Reading peer-to-peer");
  int msgSize = this->snep.read(ndefBuf, sizeof(ndefBuf));
  if (msgSize > 0) {
    NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
    msg.print();
    Serial.println("\nSuccess");
  } else {
    Serial.println("Failed");
  }
  */

  /*
  // Write peer-to-peer
  NdefMessage message = NdefMessage();
  message.addUriRecord("http://arduino.cc");

  int messageSize = message.getEncodedSize();
  if (messageSize > sizeof(ndefBuf)) {
      Serial.println("ndefBuf is too small");
      while (1) { }
  }

  message.encode(ndefBuf);
  if (0 >= this->snep.write(ndefBuf, messageSize)) {
      Serial.println("Failed");
  } else {
      Serial.println("Success");
  }
  */

  return -1;
}

int NfcClient::ReadMessage()
{
  // If reading authentication from a tag
  if (!this->nfc.tagPresent())
  {
    Serial.println("Tag not present");
    return -1;
  }

  NfcTag tag = this->nfc.read();

  if (!tag.hasNdefMessage())
  {
    Serial.println("No message");
    return -1;
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

  // TODO - Check authentication

  return 0;
}

int NfcClient::SendMessage()
{
  return 0;
}
