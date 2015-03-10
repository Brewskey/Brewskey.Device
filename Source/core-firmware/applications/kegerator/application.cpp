#include "application.h"

#include "PN532_I2C.h"
#include "PN532.h"
#include "snep.h"
#include "NfcTag.h"
#include "NdefMessage.h"

PN532_I2C pn532(Wire);
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