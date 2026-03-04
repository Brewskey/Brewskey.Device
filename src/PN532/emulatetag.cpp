/**************************************************************************/
/*!
    @file     emulatetag.cpp
    @author   Armin Wieser
    @license  BSD

    NFC Forum Type 3 Tag (FeliCa/NFCF) emulation only.
    Uses TgInitAsTarget with FeliCa params and FeliCa command loop
    (Read/Write Without Encryption) per NFC Forum Type 3 Tag 1.1.
*/
/**************************************************************************/

#include "emulatetag.h"
#include "PN532_debug.h"

#include "application.h"

// TgInitAsTarget: use MODE 5 (PICC only, Passive only) - same as original working code and Android PN532
#define TGINIT_MODE_VAL        0x05  // PICC only, Passive only = ISO14443-A target (phone can detect)
#define TGINIT_MODE            1   // byte index for mode
#define TGINIT_SENS_RES        2   // bytes 2-3 (NFC-A)
#define TGINIT_NFCID1          4   // bytes 4-6 (NFC-A)
#define TGINIT_SEL_RES         7   // byte 7 (NFC-A, 0x20 = Type 4)
#define TGINIT_NFCID2T         8   // bytes 8-15 (8 bytes, must start 01 FE)
#define TGINIT_PAD            16   // bytes 16-23 (8 bytes)
#define TGINIT_SYSTEM_CODE    24   // bytes 24-25 (2 bytes, 0xB4 0x88 for Type 3)
#define TGINIT_NFCID3T        26   // bytes 26-35 (10 bytes)
#define TGINIT_LEN_GB         36   // byte 36
#define TGINIT_LEN_HB         37   // byte 37
#define TGINIT_TOTAL_LEN      38

// Type 4 Tag (ISO7816-4 APDU) - so phones that only poll NFC-A can detect and read NDEF
#define C_APDU_CLA   0
#define C_APDU_INS   1
#define C_APDU_P1    2
#define C_APDU_P2    3
#define C_APDU_LC    4
#define C_APDU_DATA  5
#define C_APDU_P1_SELECT_BY_ID   0x00
#define ISO7816_SELECT_FILE 0xA4
#define ISO7816_READ_BINARY 0xB0
#define ISO7816_UPDATE_BINARY 0xD6
#define R_APDU_SW1_COMMAND_COMPLETE 0x90
#define R_APDU_SW2_COMMAND_COMPLETE 0x00
#define R_APDU_SW1_NDEF_TAG_NOT_FOUND 0x6a
#define R_APDU_SW2_NDEF_TAG_NOT_FOUND 0x82
#define R_APDU_SW1_FUNCTION_NOT_SUPPORTED 0x6A
#define R_APDU_SW2_FUNCTION_NOT_SUPPORTED 0x81
#define R_APDU_SW1_MEMORY_FAILURE 0x65
#define R_APDU_SW2_MEMORY_FAILURE 0x81
#define R_APDU_SW1_END_OF_FILE 0x62
#define R_APDU_SW2_END_OF_FILE 0x82
enum TagFile { NONE_Tag_File, CC_Tag_File, NDEF_Tag_File };

// FeliCa frame offsets (after length byte at 0)
#define FELICA_OFF_CMD         1
#define FELICA_OFF_IDM         2   // 8 bytes
#define FELICA_OFF_DATA       10

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

  pn532.SAMConfig();

  return true;
}

void EmulateTag::setNdefFile(const uint8_t* ndef, const int16_t ndefLength) {
  if (ndefLength > (NDEF_MAX_LENGTH - 2)) {
    DMSG("ndef file too large (> NDEF_MAX_LENGTH -2) - aborting");
    return;
  }

  ndef_file[0] = ndefLength >> 8;
  ndef_file[1] = ndefLength & 0xFF;
  memcpy(ndef_file + 2, ndef, ndefLength);
}

void EmulateTag::setUid(uint8_t* uid) {
  uidPtr = uid;
}

// Build NFC Forum Type 3 Tag attribute block (16 bytes). Block 0 of NDEF service.
void EmulateTag::buildAttributeBlock(uint8_t* out) {
  uint16_t ndefLen = (ndef_file[0] << 8) + ndef_file[1];
  // Number of 16-byte data blocks for NDEF (length header 2 + payload)
  uint16_t totalNdefBytes = 2 + ndefLen;
  uint8_t ndefBlocks = (totalNdefBytes + (FELICA_BLOCK_SIZE - 1)) / FELICA_BLOCK_SIZE;
  // Total blocks available: 1 (attribute) + ndefBlocks
  uint8_t nbr = 1 + ndefBlocks;
  uint8_t nbw = tagWriteable ? nbr : 0;

  memset(out, 0, FELICA_BLOCK_SIZE);
  out[0] = 0x00;   // NH (number of blocks in one read, 0 = 16)
  out[1] = 0x10;   // Version 1.0
  out[2] = nbr;    // Nbr
  out[3] = nbw;    // Nbw
  out[4] = nbw;    // Nbw2 (same as Nbw for NDEF)
  out[5] = tagWriteable ? 0x00 : 0xFF;  // read/write permission
  out[6] = ndef_file[0];  // NDEF length high
  out[7] = ndef_file[1];  // NDEF length low
  // bytes 8-15: type, etc. (0 = NDEF)
}

// Copy 16-byte block from our virtual block number (0 = attribute, 1..N = NDEF data) into out.
void EmulateTag::getBlock(uint16_t blockNo, uint8_t* out) {
  if (blockNo == 0) {
    buildAttributeBlock(out);
    return;
  }
  uint16_t offset = (blockNo - 1) * FELICA_BLOCK_SIZE;
  uint16_t ndefTotal = (ndef_file[0] << 8) + ndef_file[1];
  ndefTotal += 2;
  if (offset >= ndefTotal) {
    memset(out, 0, FELICA_BLOCK_SIZE);
    return;
  }
  uint8_t toCopy = ndefTotal - offset;
  if (toCopy > FELICA_BLOCK_SIZE) toCopy = FELICA_BLOCK_SIZE;
  memcpy(out, ndef_file + offset, toCopy);
  if (toCopy < FELICA_BLOCK_SIZE)
    memset(out + toCopy, 0, FELICA_BLOCK_SIZE - toCopy);
}

// Handle one FeliCa request; build response in resp (max respMaxLen). Returns response length or <0 on error.
int16_t EmulateTag::handleFelicaRequest(uint8_t* req, int16_t reqLen, uint8_t* resp, uint8_t respMaxLen) {
  if (reqLen < 10) return -1;  // at least len, cmd, IDm(8)
  uint8_t frameLen = req[0];
  uint8_t cmd = req[1];
  if (frameLen != reqLen) return -1;

  uint8_t* idm = req + 2;

  switch (cmd) {
  case FELICA_CMD_REQUEST_SERVICE: {
    // Request Service: numServices (1), serviceCodeList (2 per). We respond with version (2 bytes per service).
    if (reqLen < 11) return -1;
    uint8_t numServices = req[10];
    if (reqLen < 11 + (int16_t)(numServices * 2)) return -1;
    // Response: len, 0x03, IDm(8), numServices, versionList (2 per). Type 3 Tag version 0x0001 per service.
    uint8_t rspLen = 1 + 1 + 8 + 1 + numServices * 2;
    if (rspLen > respMaxLen) return -1;
    resp[0] = rspLen;
    resp[1] = FELICA_RSP_REQUEST_SERVICE;
    memcpy(resp + 2, idm, 8);
    resp[10] = numServices;
    for (uint8_t i = 0; i < numServices; i++) {
      resp[11 + i * 2] = 0x01;   // version LSB
      resp[11 + i * 2 + 1] = 0x00;
    }
    return rspLen;
  }

  case FELICA_CMD_READ_WITHOUT_ENC: {
    // Read: numServices(1), serviceCodeList(2*N), numBlocks(1), blockList(2*M)
    if (reqLen < 15) return -1;
    uint8_t numServices = req[10];
    int16_t off = 11 + numServices * 2;
    if (reqLen < off + 1) return -1;
    uint8_t numBlocks = req[off];
    off += 1;
    if (reqLen < off + (int16_t)(numBlocks * 2)) return -1;
    // Check service code is NDEF read (0x0F 0x09)
    if (numServices != 1 || req[11] != 0x0F || req[12] != 0x09) return -1;

    uint8_t rspLen = 1 + 1 + 8 + 2 + 1 + numBlocks * FELICA_BLOCK_SIZE;
    if (rspLen > respMaxLen) return -1;
    resp[0] = rspLen;
    resp[1] = FELICA_RSP_READ_WITHOUT_ENC;
    memcpy(resp + 2, idm, 8);
    resp[10] = 0;  // Status flag 1
    resp[11] = 0;  // Status flag 2
    resp[12] = numBlocks;
    for (uint8_t i = 0; i < numBlocks; i++) {
      uint16_t blockNo = req[off + i * 2] | (req[off + i * 2 + 1] << 8);
      getBlock(blockNo, resp + 13 + i * FELICA_BLOCK_SIZE);
    }
    return rspLen;
  }

  case FELICA_CMD_WRITE_WITHOUT_ENC: {
    if (!tagWriteable) {
      resp[0] = 1 + 1 + 8 + 2;
      resp[1] = FELICA_RSP_WRITE_WITHOUT_ENC;
      memcpy(resp + 2, idm, 8);
      resp[10] = 0x00;
      resp[11] = 0x0B;  // Error: write not allowed
      return 12;
    }
    // Write: numServices(1), serviceCodeList(2*N), numBlocks(1), blockList(2*M), then block data (16*M)
    if (reqLen < 15) return -1;
    uint8_t numServices = req[10];
    int16_t off = 11 + numServices * 2;
    if (reqLen < off + 1) return -1;
    uint8_t numBlocks = req[off];
    off += 1 + numBlocks * 2;
    if (reqLen < off + (int16_t)(numBlocks * FELICA_BLOCK_SIZE)) return -1;
    if (numServices != 1 || req[11] != 0x0B || req[12] != 0x09) return -1;  // NDEF write service

    for (uint8_t i = 0; i < numBlocks; i++) {
      uint16_t blockNo = req[11 + numServices * 2 + 1 + i * 2] | (req[11 + numServices * 2 + 1 + i * 2 + 1] << 8);
      if (blockNo == 0) continue;  // do not overwrite attribute block content we generate
      uint16_t offset = (blockNo - 1) * FELICA_BLOCK_SIZE;
      if (offset + FELICA_BLOCK_SIZE <= NDEF_MAX_LENGTH + 2) {
        memcpy(ndef_file + offset, req + off + i * FELICA_BLOCK_SIZE, FELICA_BLOCK_SIZE);
      }
    }
    tagWrittenByInitiator = true;
    uint16_t ndef_length = (ndef_file[0] << 8) + ndef_file[1];
    if (ndef_length > 0 && updateNdefCallback != 0) {
      updateNdefCallback(ndef_file + 2, ndef_length);
    }

    resp[0] = 1 + 1 + 8 + 2;
    resp[1] = FELICA_RSP_WRITE_WITHOUT_ENC;
    memcpy(resp + 2, idm, 8);
    resp[10] = 0;
    resp[11] = 0;
    return 12;
  }

  default:
    return -1;
  }
}

// Type 4 Tag (NFC-A): respond to APDU so phones that only poll ISO14443 can detect and read NDEF.
int16_t EmulateTag::handleType4Apdu(uint8_t* req, int16_t reqLen, uint8_t* resp, uint8_t respMaxLen) {
  if (reqLen < 4 || respMaxLen < 2) return -1;
  // Some stacks/PN532 may prepend a byte (e.g. logical channel); try offset 1 if first byte is 0 and second looks like INS
  int16_t off = 0;
  if (reqLen >= 5 && req[0] == 0x00 && (req[1] == ISO7816_SELECT_FILE || req[1] == ISO7816_READ_BINARY || req[1] == ISO7816_UPDATE_BINARY)) {
    off = 1;
    reqLen -= 1;
    req += 1;
  }
  uint8_t ins = req[C_APDU_INS];
  uint8_t p1 = req[C_APDU_P1];
  uint8_t p2 = req[C_APDU_P2];
  uint8_t lc = reqLen > (int16_t)C_APDU_LC ? req[C_APDU_LC] : 0;
  uint16_t p1p2_length = ((uint16_t)p1 << 8) + p2;

  if (ins == ISO7816_SELECT_FILE) {
    if (p1 == C_APDU_P1_SELECT_BY_ID && p2 == 0x0c && lc == 2) {
      uint8_t f1 = req[C_APDU_DATA];
      uint8_t f2 = req[C_APDU_DATA + 1];
      if (f1 == 0xE1 && f2 == 0x03) {
        currentFile = CC_Tag_File;
      } else if (f1 == 0xE1 && f2 == 0x04) {
        currentFile = NDEF_Tag_File;
      } else {
        resp[0] = R_APDU_SW1_NDEF_TAG_NOT_FOUND;
        resp[1] = R_APDU_SW2_NDEF_TAG_NOT_FOUND;
        return 2;
      }
      resp[0] = R_APDU_SW1_COMMAND_COMPLETE;
      resp[1] = R_APDU_SW2_COMMAND_COMPLETE;
      return 2;
    }
    if (p1 == 0x04) {  // SELECT BY NAME
      static const uint8_t ndef_app_name[] = { 0, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
      if (reqLen >= (int16_t)(C_APDU_P2 + sizeof(ndef_app_name)) && memcmp(ndef_app_name, req + C_APDU_P2, sizeof(ndef_app_name)) == 0) {
        resp[0] = R_APDU_SW1_COMMAND_COMPLETE;
        resp[1] = R_APDU_SW2_COMMAND_COMPLETE;
        return 2;
      }
      resp[0] = R_APDU_SW1_FUNCTION_NOT_SUPPORTED;
      resp[1] = R_APDU_SW2_FUNCTION_NOT_SUPPORTED;
      return 2;
    }
  }

  if (ins == ISO7816_READ_BINARY) {
    if (currentFile == NONE_Tag_File) {
      resp[0] = R_APDU_SW1_NDEF_TAG_NOT_FOUND;
      resp[1] = R_APDU_SW2_NDEF_TAG_NOT_FOUND;
      return 2;
    }
    uint8_t cc[] = {
      0, 0x0F, 0x20, 0, 0x54, 0, 0xFF, 0x04, 0x06, 0xE1, 0x04,
      (uint8_t)(NDEF_MAX_LENGTH >> 8), (uint8_t)(NDEF_MAX_LENGTH & 0xFF),
      0x00, tagWriteable ? 0x00 : 0xFF
    };
    uint8_t* data = (currentFile == CC_Tag_File) ? cc : ndef_file;
    uint16_t dataLen = (currentFile == CC_Tag_File) ? sizeof(cc) : (2 + ((ndef_file[0] << 8) + ndef_file[1]));
    if (p1p2_length >= dataLen) {
      resp[0] = R_APDU_SW1_END_OF_FILE;
      resp[1] = R_APDU_SW2_END_OF_FILE;
      return 2;
    }
    uint8_t toCopy = (lc != 0) ? lc : 0xFF;  // Le=0 often means 256
    if (p1p2_length + toCopy > dataLen) toCopy = dataLen - p1p2_length;
    if (2 + toCopy > respMaxLen) toCopy = respMaxLen - 2;
    memcpy(resp + 2, data + p1p2_length, toCopy);
    resp[0] = R_APDU_SW1_COMMAND_COMPLETE;
    resp[1] = R_APDU_SW2_COMMAND_COMPLETE;
    return 2 + toCopy;
  }

  if (ins == ISO7816_UPDATE_BINARY) {
    if (!tagWriteable) {
      resp[0] = R_APDU_SW1_FUNCTION_NOT_SUPPORTED;
      resp[1] = R_APDU_SW2_FUNCTION_NOT_SUPPORTED;
      return 2;
    }
    if (p1p2_length > NDEF_MAX_LENGTH || p1p2_length + lc > NDEF_MAX_LENGTH) {
      resp[0] = R_APDU_SW1_MEMORY_FAILURE;
      resp[1] = R_APDU_SW2_MEMORY_FAILURE;
      return 2;
    }
    if (reqLen < C_APDU_DATA + lc) return -1;
    memcpy(ndef_file + p1p2_length, req + C_APDU_DATA, lc);
    tagWrittenByInitiator = true;
    uint16_t ndef_length = (ndef_file[0] << 8) + ndef_file[1];
    if (ndef_length > 0 && updateNdefCallback != 0) {
      updateNdefCallback(ndef_file + 2, ndef_length);
    }
    resp[0] = R_APDU_SW1_COMMAND_COMPLETE;
    resp[1] = R_APDU_SW2_COMMAND_COMPLETE;
    return 2;
  }

  resp[0] = R_APDU_SW1_FUNCTION_NOT_SUPPORTED;
  resp[1] = R_APDU_SW2_FUNCTION_NOT_SUPPORTED;
  return 2;
}

bool EmulateTag::emulate(const uint16_t tgInitAsTargetTimeout) {
  // Exact layout that worked in original code: MODE 5, SENS_RES, NFCID1, SEL_RES, FeliCa zeros, NFCID3t zeros
  uint8_t command[TGINIT_TOTAL_LEN];
  memset(command, 0, sizeof(command));
  command[0] = PN532_COMMAND_TGINITASTARGET;
  command[TGINIT_MODE] = TGINIT_MODE_VAL;  // 0x05 = PICC only, Passive only (ISO14443-A)
  command[TGINIT_SENS_RES] = 0x04;
  command[TGINIT_SENS_RES + 1] = 0x00;
  if (uidPtr != 0) {
    memcpy(command + TGINIT_NFCID1, uidPtr, 3);
  }
  command[TGINIT_SEL_RES] = 0x20;  // Type 4
  // FeliCa params: leave zero (PN532 in mode 5 responds as NFC-A only; zeros match original)
  // bytes 8-35 already zeroed by memset
  command[TGINIT_LEN_GB] = 0;
  command[TGINIT_LEN_HB] = 0;

  if (1 != pn532.tgInitAsTarget(command, sizeof(command), tgInitAsTargetTimeout)) {
    DMSG("tgInitAsTarget failed or timed out!\r\n");
    return false;
  }

  tagWrittenByInitiator = false;
  currentFile = NONE_Tag_File;

  uint8_t rwbuf[128];
  uint8_t respbuf[128];
  bool firstRead = true;

  while (true) {
    int16_t status = pn532.tgGetData(rwbuf, sizeof(rwbuf), FELICA_TGGETDATA_TIMEOUT_MS);

    if (status < 0) {
      DMSG("tgGetData failed!\r\n");
      pn532.inRelease();
      return !firstRead;
    }
    firstRead = false;

    // With MODE 5 we only get NFC-A / Type 4 APDU. FeliCa check kept for clarity if mode ever changes.
    bool looksLikeFelica = (status >= 10 && rwbuf[0] == (uint8_t)status && rwbuf[1] >= 0x02 && rwbuf[1] <= 0x08);
    int16_t rspLen;
    if (looksLikeFelica) {
      rspLen = handleFelicaRequest(rwbuf, (int16_t)status, respbuf, sizeof(respbuf));
    } else {
      rspLen = handleType4Apdu(rwbuf, (int16_t)status, respbuf, sizeof(respbuf));
    }
    if (rspLen < 0) {
      // Send generic "not supported" so reader doesn't hang waiting for response
      respbuf[0] = R_APDU_SW1_FUNCTION_NOT_SUPPORTED;
      respbuf[1] = R_APDU_SW2_FUNCTION_NOT_SUPPORTED;
      rspLen = 2;
    }

    if (!pn532.tgSetData(respbuf, (uint8_t)rspLen)) {
      DMSG("tgSetData failed\r\n");
      pn532.inRelease();
      return true;
    }
  }
}
