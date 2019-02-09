#ifndef NfcAdapter_h
#define NfcAdapter_h

#include "PN532/PN532Interface.h"
#include "PN532/PN532.h"
#include "PN532/tag_information.h"
#include "NDEF/NfcTag.h"
#include "NDEF/Ndef.h"

// Drivers
#include "MifareClassic.h"
#include "MifareUltralight.h"

#define TAG_TYPE_MIFARE_CLASSIC (0)
#define TAG_TYPE_1 (1)
#define TAG_TYPE_2 (2)
#define TAG_TYPE_3 (3)
#define TAG_TYPE_4 (4)
#define TAG_TYPE_UNKNOWN (99)

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

class NfcAdapter {
public:
  NfcAdapter(PN532Interface &interface);

  ~NfcAdapter(void);
  void begin(boolean verbose = true);
  boolean tagPresent(unsigned long timeout = 0); // tagAvailable
  NfcTag read();
  boolean write(NdefMessage& ndefMessage);
  // erase tag by writing an empty NDEF record
  boolean erase();
  // format a tag as NDEF
  boolean format();
  // reset tag back to factory state
  boolean clean();
private:
  PN532 * shield;
  unsigned int guessTagType();
  TagInformation tagInfo;
};

#endif
