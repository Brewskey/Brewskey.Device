#include "NfcClient.h"

NfcClient::NfcClient() :
#ifdef SPI_HW_MODE
  pn532spi(SPI, SS),
#else
  pn532spi(SCK, MISO, MOSI, SS),
#endif
  nfc(pn532spi)
{
  nfc.init();

  message = NdefMessage();
  message.addUriRecord("http://www.seeedstudio.com");
  messageSize = message.getEncodedSize();
  if (messageSize > sizeof(ndefBuf)) {
      Serial.println("ndefBuf is too small");
      while (1) { }
  }

  Serial.print("Ndef encoded message size: ");
  Serial.println(messageSize);

  message.encode(ndefBuf);

  // comment out this command for no ndef message
  nfc.setNdefFile(ndefBuf, messageSize);

  // uid must be 3 bytes!
  nfc.setUid(uid);
}

NfcState::value NfcClient::Tick()
{
  nfc.emulate();

  delay(1000);

  return NfcState::NO_MESSAGE;
  return this->SendMessage();
}

NfcState::value NfcClient::ReadMessage()
{
  /*
#ifndef SNEP
  // If reading authentication from a tag
  if (!this->nfc.tagPresent())
  {
    Serial.println("Tag not present");
    return NfcState::ERROR;
  }

  NfcTag tag = this->nfc.read();

  if (!tag.hasNdefMessage())
  {
    Serial.println("No message");
    return NfcState::ERROR;
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
  Serial.println(authenticationKey);
  Serial.println("printed");
#endif */
  return NfcState::NO_MESSAGE;
}

NfcState::value NfcClient::SendMessage()
{
  /*
  if (nfc.tagPresent()) {
      NdefMessage message = NdefMessage();
      message.addUriRecord("http://arduino.cc");

      bool success = nfc.write(message);
      if (success) {
        Serial.println("Success. Try reading this tag with your phone.");
      } else {
        Serial.println("Write failed.");
      }
  }
  */

  return NfcState::NO_MESSAGE;
}
