#ifndef Type4Tag_h
#define Type4Tag_h

#include "PN532/PN532.h"
#include "NfcTag.h"
#include "Ndef.h"

#define NFC_FORUM_TYPE_4 ("NFC Forum Type 4")

/**
 * Read NDEF from ISO 14443-4 (Type 4) tags and HCE devices.
 * Uses APDU: SELECT NDEF AID, SELECT NDEF file, READ BINARY.
 * Compatible with phone HCE presenting NDEF (e.g. Brewskey pour flow).
 */
class Type4Tag
{
public:
  Type4Tag(PN532& nfcShield);
  ~Type4Tag();
  NfcTag read(byte *uid, unsigned int uidLength);

private:
  PN532* nfc;
  static const uint8_t NDEF_AID[];
  static const uint8_t NDEF_AID_LEN;
  static const uint8_t FILE_NDEF_E104[];
  static const uint8_t FILE_NDEF_E104_LEN;
  static const uint8_t MAX_NDEF_SIZE;

  bool selectNdefApp();
  bool selectNdefFile();
  int readNdefFile(byte* out, int maxLen);
};

#endif
