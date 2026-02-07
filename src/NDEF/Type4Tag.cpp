#include "Type4Tag.h"

#define T4T_DEBUG 0

// NFC Forum NDEF Tag Application AID (NFC Forum Type 4 Tag Operation 1.0)
const uint8_t Type4Tag::NDEF_AID[] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
const uint8_t Type4Tag::NDEF_AID_LEN = 7;

// NDEF file ID E104 (common for Type 4 NDEF file)
const uint8_t Type4Tag::FILE_NDEF_E104[] = { 0xE1, 0x04 };
const uint8_t Type4Tag::FILE_NDEF_E104_LEN = 2;

const uint8_t Type4Tag::MAX_NDEF_SIZE = 255;

Type4Tag::Type4Tag(PN532& nfcShield)
{
  nfc = &nfcShield;
}

Type4Tag::~Type4Tag()
{
}

bool Type4Tag::selectNdefApp()
{
  uint8_t apdu[256];
  uint8_t len;

  // SELECT by AID: CLA=00, INS=A4, P1=04, P2=00, Lc=07, AID
  apdu[0] = 0x00;
  apdu[1] = 0xA4;
  apdu[2] = 0x04;
  apdu[3] = 0x00;
  apdu[4] = NDEF_AID_LEN;
  memcpy(apdu + 5, NDEF_AID, NDEF_AID_LEN);
  len = 5 + NDEF_AID_LEN;

  uint8_t response[64];
  uint8_t responseLen = sizeof(response);

  if (!nfc->inDataExchange(apdu, len, response, &responseLen)) {
#if T4T_DEBUG
    Serial.println(F("Type4: SELECT AID failed"));
#endif
    return false;
  }

  // Success: R-APDU ends with 90 00 (or 61 XX for GET RESPONSE)
  if (responseLen >= 2 && response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
    return true;
  }
  if (responseLen >= 2 && response[responseLen - 2] == 0x61) {
    // GET RESPONSE not implemented here; try next step
    return true;
  }
  return false;
}

bool Type4Tag::selectNdefFile()
{
  uint8_t apdu[16];
  // SELECT file by ID: CLA=00, INS=A4, P1=00, P2=0C, Lc=02, file ID E104
  apdu[0] = 0x00;
  apdu[1] = 0xA4;
  apdu[2] = 0x00;
  apdu[3] = 0x0C;
  apdu[4] = FILE_NDEF_E104_LEN;
  apdu[5] = FILE_NDEF_E104[0];
  apdu[6] = FILE_NDEF_E104[1];

  uint8_t response[64];
  uint8_t responseLen = sizeof(response);

  if (!nfc->inDataExchange(apdu, 7, response, &responseLen)) {
#if T4T_DEBUG
    Serial.println(F("Type4: SELECT file failed"));
#endif
    return false;
  }

  if (responseLen >= 2 && response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
    return true;
  }
  return false;
}

int Type4Tag::readNdefFile(byte* out, int maxLen)
{
  uint8_t apdu[5];
  // READ BINARY: CLA=00, INS=B0, P1=00, P2=00, Le=MAX_NDEF_SIZE
  apdu[0] = 0x00;
  apdu[1] = 0xB0;
  apdu[2] = 0x00;
  apdu[3] = 0x00;
  apdu[4] = (maxLen > 255) ? 255 : (uint8_t)maxLen;

  uint8_t response[260];
  uint8_t responseLen = sizeof(response);

  if (!nfc->inDataExchange(apdu, 5, response, &responseLen)) {
#if T4T_DEBUG
    Serial.println(F("Type4: READ BINARY failed"));
#endif
    return -1;
  }

  // R-APDU: data + 90 00
  if (responseLen < 2) return -1;
  if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00) {
    return -1;
  }

  int dataLen = responseLen - 2;
  if (dataLen > maxLen) dataLen = maxLen;
  memcpy(out, response, dataLen);
  return dataLen;
}

NfcTag Type4Tag::read(byte *uid, unsigned int uidLength)
{
  if (!selectNdefApp()) {
    return NfcTag(uid, uidLength, NFC_FORUM_TYPE_4);
  }
  if (!selectNdefFile()) {
    return NfcTag(uid, uidLength, NFC_FORUM_TYPE_4);
  }

  byte ndefBuf[MAX_NDEF_SIZE];
  int ndefLen = readNdefFile(ndefBuf, MAX_NDEF_SIZE);
  if (ndefLen <= 0) {
    return NfcTag(uid, uidLength, NFC_FORUM_TYPE_4);
  }

  // Parse NDEF message (NdefMessage constructor accepts raw bytes)
  NdefMessage message = NdefMessage(ndefBuf, ndefLen);
  return NfcTag(uid, uidLength, NFC_FORUM_TYPE_4, ndefBuf, ndefLen);
}
