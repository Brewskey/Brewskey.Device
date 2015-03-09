#include "application.h"

#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"

PN532_SPI pn532spi(SPI, SS);
NfcAdapter nfc = NfcAdapter(pn532spi);

void setup(void) {
    Serial.begin(115200);
    while(!Serial.available()) {
      Spark.process();
    }

    Serial.println("NDEF Reader");
    nfc.begin();
}

void loop(void) {
  return;
    Serial.println("\nScan a NFC tag\n");
    if (nfc.tagPresent())
    {
        NfcTag tag = nfc.read();
        tag.print();
    }
    delay(5000);
}

/*
#include "PN532_HSU.h"
#include "PN532.h"
#include "snep.h"
#include "NdefMessage.h"

PN532_HSU pn532(Serial1);
SNEP nfc(pn532);
uint8_t ndefBuf[128];

void setup(void) {
  Serial.begin(115200); // Make sure your serial terminal is closed before power the Core.
  while(!Serial.available()) {
    Spark.process();
  }
  Serial.println("Hello!");
}

void loop() {
    Serial.println("Waiting for message from Peer");
    int msgSize = nfc.read(ndefBuf, sizeof(ndefBuf));
    if (msgSize > 0) {
        NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
        msg.print();
        Serial.println("\nSuccess");
    } else {
        Serial.println("Failed");
    }
    delay(3000);
}
*/
