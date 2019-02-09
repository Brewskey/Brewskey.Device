#include "NfcAdapter.h"

NfcAdapter::NfcAdapter(PN532Interface &interface)
{
  shield = new PN532(interface);
}

NfcAdapter::~NfcAdapter(void)
{
  delete shield;
}

void NfcAdapter::begin(boolean verbose)
{
  shield->begin();

  uint32_t versiondata = shield->getFirmwareVersion();

  if (!versiondata)
  {
    Serial.print(F("Didn't find PN53x board"));
    while (1) {
      Particle.process();
    } // halt
  }

  if (verbose)
  {
    Serial.print(F("Found chip PN5")); Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print(F("Firmware ver. ")); Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  }
  // configure board to read RFID tags
  shield->SAMConfig();
}

boolean NfcAdapter::tagPresent(unsigned long timeout)
{
  uint8_t success;

  if (timeout == 0)
  {
    success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, &tagInfo);
  }
  else
  {
    success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, &tagInfo, timeout);
  }
  return success;
}

boolean NfcAdapter::erase()
{
  boolean success;
  NdefMessage message = NdefMessage();
  message.addEmptyRecord();
  return write(message);
}

boolean NfcAdapter::format()
{
  boolean success;
  if (tagInfo.GetTagType() == TagInformation::MIFARE_CLASSIC)
  {
    MifareClassic mifareClassic = MifareClassic(*shield);
    success = mifareClassic.formatNDEF(&tagInfo);
  }
  else
  {
    Serial.print(F("Unsupported Tag."));
    success = false;
  }
  return success;
}

boolean NfcAdapter::clean()
{
  uint8_t type = guessTagType();

  if (type == TAG_TYPE_MIFARE_CLASSIC)
  {
#ifdef NDEF_DEBUG
    Serial.println(F("Cleaning Mifare Classic"));
#endif
    MifareClassic mifareClassic = MifareClassic(*shield);
    return mifareClassic.formatMifare(tagInfo.uid, tagInfo.uidLength);
  }
  else if (type == TAG_TYPE_2)
  {
#ifdef NDEF_DEBUG
    Serial.println(F("Cleaning Mifare Ultralight"));
#endif
    MifareUltralight ultralight = MifareUltralight(*shield);
    return ultralight.clean();
  }
  else if (type == TAG_TYPE_4) {
    // TODO - Use DESFire code
  }
  else
  {
    Serial.print(F("No driver for card type ")); Serial.println(type);
    return false;
  }

}

NfcTag NfcAdapter::read()
{
  uint8_t type = guessTagType();

  if (type == TAG_TYPE_MIFARE_CLASSIC)
  {
#ifdef NDEF_DEBUG
    Serial.println(F("Reading Mifare Classic"));
#endif
    MifareClassic mifareClassic = MifareClassic(*shield);
    return mifareClassic.read(tagInfo.uid, tagInfo.uidLength);
  }
  else if (type == TAG_TYPE_2)
  {
#ifdef NDEF_DEBUG
    Serial.println(F("Reading Mifare Ultralight"));
#endif
    MifareUltralight ultralight = MifareUltralight(*shield);
    return ultralight.read(tagInfo.uid, tagInfo.uidLength);
  }
  else if (type == TAG_TYPE_4) {
    // TODO - Use DESFire code
  }
  else if (type == TAG_TYPE_UNKNOWN)
  {
    Serial.print(F("Can not determine tag type"));
    return NfcTag(tagInfo.uid, tagInfo.uidLength);
  }
  else
  {
    Serial.print(F("No driver for card type ")); Serial.println(type);
    // TODO should set type here
    return NfcTag(tagInfo.uid, tagInfo.uidLength);
  }

}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
  boolean success;
  uint8_t type = guessTagType();

  if (type == TAG_TYPE_MIFARE_CLASSIC)
  {
#ifdef NDEF_DEBUG
    Serial.println(F("Writing Mifare Classic"));
#endif
    MifareClassic mifareClassic = MifareClassic(*shield);
    success = mifareClassic.write(ndefMessage, tagInfo.uid, tagInfo.uidLength);
  }
  else if (type == TAG_TYPE_2)
  {
#ifdef NDEF_DEBUG
    Serial.println(F("Writing Mifare Ultralight"));
#endif
    MifareUltralight mifareUltralight = MifareUltralight(*shield);
    success = mifareUltralight.write(ndefMessage, tagInfo.uid, tagInfo.uidLength);
  }
  else if (type == TAG_TYPE_4) {
    // TODO - Use DESFire code
  }
  else if (type == TAG_TYPE_UNKNOWN)
  {
    Serial.print(F("Can not determine tag type"));
    success = false;
  }
  else
  {
    Serial.print(F("No driver for card type ")); Serial.println(type);
    success = false;
  }

  return success;
}

// TODO this should return a Driver MifareClassic, MifareUltralight, Type 4, Unknown
// Guess Tag Type by looking at the ATQA and SAK values
// Need to follow spec for Card Identification. Maybe AN1303, AN1305 and ???
unsigned int NfcAdapter::guessTagType()
{
  switch (this->tagInfo.GetTagType()) {
    case TagInformation::DESFIRE_EV1:
    case TagInformation::DESFIRE_EV1_RANDOM: {
      return TAG_TYPE_4;
    }

    case TagInformation::MIFARE_CLASSIC: {
      return TAG_TYPE_MIFARE_CLASSIC;
    }

    case TagInformation::MIFARE_ULTRALIGHT: {
      return TAG_TYPE_2;
    }

    default: {
      return TAG_TYPE_UNKNOWN;
    }
  }
}
