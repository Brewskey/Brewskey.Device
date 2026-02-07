/**************************************************************************/
/*!
    @file     emulatetag.cpp
    @author   Armin Wieser
    @license  BSD
*/
/**************************************************************************/

#include "emulatetag.h"
#include "PN532_debug.h"

#include "application.h"

#define MAX_TGREAD


// Command APDU
#define C_APDU_CLA   0
#define C_APDU_INS   1 // instruction
#define C_APDU_P1    2 // parameter 1
#define C_APDU_P2    3 // parameter 2
#define C_APDU_LC    4 // length command
#define C_APDU_DATA  5 // data

#define C_APDU_P1_SELECT_BY_ID   0x00
#define C_APDU_P1_SELECT_BY_FILE_ID 0x02  // alternate P1 for select by file identifier
#define C_APDU_P1_SELECT_BY_NAME 0x04

// Response APDU
#define R_APDU_SW1_COMMAND_COMPLETE 0x90
#define R_APDU_SW2_COMMAND_COMPLETE 0x00

#define R_APDU_SW1_NDEF_TAG_NOT_FOUND 0x6a
#define R_APDU_SW2_NDEF_TAG_NOT_FOUND 0x82

#define R_APDU_SW1_FUNCTION_NOT_SUPPORTED 0x6A
#define R_APDU_SW2_FUNCTION_NOT_SUPPORTED 0x81

#define R_APDU_SW1_MEMORY_FAILURE 0x65
#define R_APDU_SW2_MEMORY_FAILURE 0x81

#define R_APDU_SW1_END_OF_FILE_BEFORE_REACHED_LE_BYTES 0x62
#define R_APDU_SW2_END_OF_FILE_BEFORE_REACHED_LE_BYTES 0x82

// ISO7816-4 commands
#define ISO7816_SELECT_FILE 0xA4
#define ISO7816_READ_BINARY 0xB0
#define ISO7816_UPDATE_BINARY 0xD6

typedef enum { NONE_Tag_File, CC_Tag_File, NDEF_Tag_File } tag_file;   // CC ... Compatibility Container

// READ BINARY: max data bytes in one response (rwbuf size 128 minus 2 for SW1-SW2)
#define READ_BINARY_MAX_RESPONSE_DATA 126
// Longer timeout for tag emulation so slower phones can complete the APDU sequence
#define EMULATE_TGGETDATA_TIMEOUT_MS 500

bool EmulateTag::init() {
  pn532.begin();
  uint32_t versiondata = pn532.getFirmwareVersion();

  if (!versiondata)
  {
    Serial.print(F("Didn't find PN53x board"));
    return false;
  }

  Serial.print(F("Found chip PN5")); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print(F("Firmware ver. ")); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags (for some reason this returns 0 with SPI)
  pn532.SAMConfig();

  return true;
}

void EmulateTag::setNdefFile(const uint8_t* ndef, const int16_t ndefLength) {
  if (ndefLength > (NDEF_MAX_LENGTH - 2)) {
    DMSG("ndef file too large (> NDEF_MAX_LENGHT -2) - aborting");
    return;
  }

  ndef_file[0] = ndefLength >> 8;
  ndef_file[1] = ndefLength & 0xFF;
  memcpy(ndef_file + 2, ndef, ndefLength);
}

void EmulateTag::setUid(uint8_t* uid) {
  uidPtr = uid;
}

bool EmulateTag::emulate(const uint16_t tgInitAsTargetTimeout) {
  // http://www.nxp.com/documents/application_note/AN133910.pdf
  // NFCID1: 4 bytes with first byte 0x08 (random UID per ISO 14443-3) for phone compatibility
  uint8_t command[] = {
      PN532_COMMAND_TGINITASTARGET,
      5, // MODE: PICC only, Passive only

      0x04, 0x00,             // SENS_RES
      0x08, 0x00, 0x00, 0x00, // NFCID1 (4 bytes; byte 0 = 0x08)
      0x20,                   // SEL_RES (4-byte UID, Type 4 / ISO-DEP)

      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, // FeliCaParams
      0, 0,

      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // NFCID3t

      0, // length of general bytes
      0  // length of historical bytes
  };

  if (uidPtr != 0) {  // bytes 1-3 of NFCID1 from uid
    command[5] = uidPtr[0];
    command[6] = uidPtr[1];
    command[7] = uidPtr[2];
  }

  if (1 != pn532.tgInitAsTarget(command, sizeof(command), tgInitAsTargetTimeout)) {
    DMSG("tgInitAsTarget failed or timed out!\r\n");
    return false;
  }

  // NFC Forum Type 4 Tag Compatibility Container (file E103). Layout per Type 4 Tag spec:
  // Bytes 0-1: Version (00 0F = 1.0); T=20 block; File Control TLV T=04 L=06, FileId E104, max size, read/write access.
  uint8_t compatibility_container[] = {
    0x00, 0x0F,                                 // Version 1.0
    0x20,                                       // T=20 (Type 4 Tag capability)
    0x00, 0x54,                                 // Length/capability
    0x00, 0xFF,                                 // Max NDEF size (0xFF = 255) / MLe, MLc
    0x04, 0x06, 0xE1, 0x04,                     // File Control TLV: T=04, L=06, FileId E104 (NDEF file)
    ((NDEF_MAX_LENGTH & 0xFF00) >> 8), (NDEF_MAX_LENGTH & 0xFF), // max NDEF file size
    0x00,                                       // read access 0x00 = granted
    0x00                                        // write access 0x00 = granted | 0xFF = deny
  };

  if (tagWriteable == false) {
    compatibility_container[14] = 0xFF;
  }

  tagWrittenByInitiator = false;

  uint8_t rwbuf[128];
  uint8_t sendlen;
  int16_t status;
  int16_t apduLen = 0;  // received APDU length from tgGetData
  tag_file currentFile = NONE_Tag_File;
  uint16_t cc_size = sizeof(compatibility_container);
  bool runLoop = true;
  bool firstRead = true;
  int emptyResultCount = 0;

  while (runLoop) {
    status = pn532.tgGetData(rwbuf, sizeof(rwbuf), EMULATE_TGGETDATA_TIMEOUT_MS);
    apduLen = status;

    if (status < 0) {
      DMSG("tgGetData failed!\r\n");
      pn532.inRelease();
      return !firstRead;
    }

    firstRead = false;

    // Minimum APDU: CLA, INS, P1, P2 (4 bytes). Reject shorter to avoid out-of-bounds reads.
    if (apduLen < 4) {
      DMSG("APDU too short\r\n");
      setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
      if (!pn532.tgSetData(rwbuf, sendlen)) {
        pn532.inRelease();
        return true;
      }
      continue;
    }

    uint8_t p1 = rwbuf[C_APDU_P1];
    uint8_t p2 = rwbuf[C_APDU_P2];
    uint8_t lc = rwbuf[C_APDU_LC];
    uint16_t p1p2_length = ((int16_t)p1 << 8) + p2;

    // libnfc bails here but we'll look for 3 empty read binary commands since
    // we have to constantly read from buffer.
    /*if (p1p2_length < 4) {
      emptyResultCount++;
      if (emptyResultCount > 6) {
        pn532.inRelease();
        return true;
      }
    } else {
      emptyResultCount = 0;
    }*/

    switch (rwbuf[C_APDU_INS]) {
    case ISO7816_SELECT_FILE:
      switch (p1) {
      case C_APDU_P1_SELECT_BY_ID:
      case C_APDU_P1_SELECT_BY_FILE_ID:
        // P2=0x0C (no FCI) or 0x00; some readers use 0x00 for select by file ID
        if (p2 != 0x0c && p2 != 0x00) {
          DMSG("C_APDU_P2 not 0x0c/0x00\r\n");
          setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
        }
        else if (apduLen < 7) {
          setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
        }
        else if (lc == 2 && rwbuf[C_APDU_DATA] == 0xE1 && (rwbuf[C_APDU_DATA + 1] == 0x03 || rwbuf[C_APDU_DATA + 1] == 0x04)) {
          setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
          if (rwbuf[C_APDU_DATA + 1] == 0x03) {
            currentFile = CC_Tag_File;
          }
          else if (rwbuf[C_APDU_DATA + 1] == 0x04) {
            currentFile = NDEF_Tag_File;
          }
        }
        else {
          setResponse(TAG_NOT_FOUND, rwbuf, &sendlen);
        }
        break;
      case C_APDU_P1_SELECT_BY_NAME: {
        // AID is in command data at C_APDU_DATA (7 bytes). Require full APDU length 5 + 7.
        if (apduLen < 12 || lc < 7) {
          setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
        }
        else {
          const uint8_t ndef_aid_v2[] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
          const uint8_t ndef_aid_v1[] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00 };
          if (0 == memcmp(ndef_aid_v2, rwbuf + C_APDU_DATA, 7) || 0 == memcmp(ndef_aid_v1, rwbuf + C_APDU_DATA, 7)) {
            setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
          }
          else {
            DMSG("function not supported\r\n");
            setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
          }
        }
        break;
      }
      default:
        setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
        break;
      }
      break;
    case ISO7816_READ_BINARY: {
      // For READ BINARY the byte at C_APDU_LC is Le (expected length). Le=0 means 256 (ISO 7816-4).
      // If APDU has only 4 bytes (no Le), treat as request maximum.
      uint16_t le = 256;
      if (apduLen >= 5) {
        le = rwbuf[C_APDU_LC];
        if (le == 0)
          le = 256;
      }
      switch (currentFile) {
      case NONE_Tag_File:
        setResponse(TAG_NOT_FOUND, rwbuf, &sendlen);
        break;
      case CC_Tag_File: {
        uint16_t cc_remaining = (p1p2_length >= cc_size) ? 0 : (cc_size - p1p2_length);
        uint16_t readLen = le;
        if (readLen > cc_remaining)
          readLen = cc_remaining;
        if (readLen > READ_BINARY_MAX_RESPONSE_DATA)
          readLen = READ_BINARY_MAX_RESPONSE_DATA;
        if (readLen > 0) {
          memcpy(rwbuf, compatibility_container + p1p2_length, readLen);
          setResponse(COMMAND_COMPLETE, rwbuf + readLen, &sendlen, readLen);
        } else {
          setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
        }
        break;
      }
      case NDEF_Tag_File: {
        uint16_t ndef_file_len = 2 + ((ndef_file[0] << 8) + ndef_file[1]);
        uint16_t ndef_remaining = (p1p2_length >= ndef_file_len) ? 0 : (ndef_file_len - p1p2_length);
        uint16_t readLen = le;
        if (readLen > ndef_remaining)
          readLen = ndef_remaining;
        if (readLen > READ_BINARY_MAX_RESPONSE_DATA)
          readLen = READ_BINARY_MAX_RESPONSE_DATA;
        if (readLen > 0) {
          memcpy(rwbuf, ndef_file + p1p2_length, readLen);
          setResponse(COMMAND_COMPLETE, rwbuf + readLen, &sendlen, readLen);
        } else {
          setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
        }
        break;
      }
      }
      break;
    }
    case ISO7816_UPDATE_BINARY:
      if (!tagWriteable) {
        setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
      }
      else if (apduLen < 5 || (uint16_t)(5 + lc) > (uint16_t)apduLen) {
        setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
      }
      else if (p1p2_length > NDEF_MAX_LENGTH || lc > NDEF_MAX_LENGTH || p1p2_length + lc > NDEF_MAX_LENGTH) {
        setResponse(MEMORY_FAILURE, rwbuf, &sendlen);
      }
      else {
        memcpy(ndef_file + p1p2_length, rwbuf + C_APDU_DATA, lc);
        setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
        tagWrittenByInitiator = true;

        uint16_t ndef_length = (ndef_file[0] << 8) + ndef_file[1];
        if ((ndef_length > 0) && (updateNdefCallback != 0)) {
          updateNdefCallback(ndef_file + 2, ndef_length);
        }
      }
      break;
    default:
      DMSG("Command not supported!");
      DMSG_HEX(rwbuf[C_APDU_INS]);
      DMSG("\r\n");
      setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
      break;
    }

    if (!pn532.tgSetData(rwbuf, sendlen)) {
      DMSG("tgSetData failed\r\n");
      pn532.inRelease();
      return true;
    }
  }

  DMSG("Emulation finished\r\n");
  pn532.inRelease();
  return true;
}

void EmulateTag::setResponse(responseCommand cmd, uint8_t* buf, uint8_t* sendlen, uint8_t sendlenOffset) {
  switch (cmd) {
  case COMMAND_COMPLETE:
    buf[0] = R_APDU_SW1_COMMAND_COMPLETE;
    buf[1] = R_APDU_SW2_COMMAND_COMPLETE;
    *sendlen = 2 + sendlenOffset;
    break;
  case TAG_NOT_FOUND:
    buf[0] = R_APDU_SW1_NDEF_TAG_NOT_FOUND;
    buf[1] = R_APDU_SW2_NDEF_TAG_NOT_FOUND;
    *sendlen = 2;
    break;
  case FUNCTION_NOT_SUPPORTED:
    buf[0] = R_APDU_SW1_FUNCTION_NOT_SUPPORTED;
    buf[1] = R_APDU_SW2_FUNCTION_NOT_SUPPORTED;
    *sendlen = 2;
    break;
  case MEMORY_FAILURE:
    buf[0] = R_APDU_SW1_MEMORY_FAILURE;
    buf[1] = R_APDU_SW2_MEMORY_FAILURE;
    *sendlen = 2;
    break;
  case END_OF_FILE_BEFORE_REACHED_LE_BYTES:
    buf[0] = R_APDU_SW1_END_OF_FILE_BEFORE_REACHED_LE_BYTES;
    buf[1] = R_APDU_SW2_END_OF_FILE_BEFORE_REACHED_LE_BYTES;
    *sendlen = 2;
    break;
  }
}
