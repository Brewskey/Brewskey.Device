#include "NfcClient.h"

NfcClient::NfcClient() :
  pn532spi(SCK, MISO, MOSI, SS), nfc(pn532spi), snep(pn532spi)
{
  this->nfc.begin();
}

void NfcClient::Tick()
{
  // If reading authentication from a tag
  if (!this->nfc.tagPresent())
  {
    return;
  }

  NfcTag tag = this->nfc.read();

  if (!tag.hasNdefMessage())
  {
    return;
  }

  int totalLength = 0;
  NdefMessage message = tag.getNdefMessage();
  int recordCount = message.getRecordCount();

  String authenticationKey;

  for (int i = 0; i < recordCount; i++) {
    NdefRecord record = message[i];

    totalLength += record.getPayloadLength();

    byte* payload;
    record.getPayload(payload);

    authenticationKey += String(reinterpret_cast< char const* >(payload));
  }

  Serial.println(authenicationKey);

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
}
