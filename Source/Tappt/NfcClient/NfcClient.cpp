#include "NfcClient.h"

NfcClient::NfcClient() :
  pn532spi(SCK, MISO, MOSI, SS), nfc(pn532spi)//, snep(pn532spi)
{
  this->nfc.begin();
}

void NfcClient::Tick()
{
  // If reading authentication from a tag
  if (this->nfc.tagPresent())
  {
      NfcTag tag = this->nfc.read();
      tag.print();
  }

  /*
  // Read message over peer-to-peer
  int msgSize = this->snep.read(ndefBuf, sizeof(ndefBuf));
  if (msgSize > 0) {
    NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
    msg.print();
    Serial.println("\nSuccess");
  } else {
    Serial.println("Failed");
  }

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

/*
Serial.println("\r\nScan a NFC tag\r\n");
if (nfc.tagPresent())
{

    NfcTag tag = nfc.read();
    tag.print();
}
*/
