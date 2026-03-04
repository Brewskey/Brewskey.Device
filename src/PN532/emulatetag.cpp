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

// TgInitAsTarget layout for FeliCa (NFCF) only
#define TGINIT_MODE            1   // byte 1: 0x01 = Passive only (FeliCa when FeliCa params present)
#define TGINIT_SENS_RES        2   // bytes 2-3
#define TGINIT_NFCID1          4   // bytes 4-6
#define TGINIT_SEL_RES         7   // byte 7
#define TGINIT_NFCID2T         8   // bytes 8-15 (8 bytes, must start 01 FE)
#define TGINIT_PAD            16   // bytes 16-23 (8 bytes)
#define TGINIT_SYSTEM_CODE    24   // bytes 24-25 (2 bytes, 0xB4 0x88 for Type 3)
#define TGINIT_NFCID3T        26   // bytes 26-35 (10 bytes)
#define TGINIT_LEN_GB         36   // byte 36
#define TGINIT_LEN_HB         37   // byte 37
#define TGINIT_TOTAL_LEN      38

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

bool EmulateTag::emulate(const uint16_t tgInitAsTargetTimeout) {
  uint8_t command[TGINIT_TOTAL_LEN];
  memset(command, 0, sizeof(command));
  command[0] = PN532_COMMAND_TGINITASTARGET;
  command[1] = TGINIT_MODE;  // Passive only (FeliCa when FeliCa params set)
  // bytes 2-7: SENS_RES, NFCID1, SEL_RES = 0
  command[TGINIT_NFCID2T + 0] = 0x01;
  command[TGINIT_NFCID2T + 1] = 0xFE;
  if (uidPtr != 0) {
    memcpy(command + TGINIT_NFCID2T + 2, uidPtr, 6);
  } else {
    command[TGINIT_NFCID2T + 2] = 0x00;
    command[TGINIT_NFCID2T + 3] = 0x00;
    command[TGINIT_NFCID2T + 4] = 0x00;
    command[TGINIT_NFCID2T + 5] = 0x00;
    command[TGINIT_NFCID2T + 6] = 0x00;
    command[TGINIT_NFCID2T + 7] = 0x00;
  }
  // PAD: 8 bytes zero (already zeroed)
  command[TGINIT_SYSTEM_CODE] = (TYPE3_SYSTEM_CODE & 0xFF);       // 0xB4
  command[TGINIT_SYSTEM_CODE + 1] = (TYPE3_SYSTEM_CODE >> 8);      // 0x88
  // NFCID3t: first 8 bytes same as NFCID2t, then 2 bytes (e.g. 0x00)
  memcpy(command + TGINIT_NFCID3T, command + TGINIT_NFCID2T, 8);
  command[TGINIT_LEN_GB] = 0;
  command[TGINIT_LEN_HB] = 0;

  if (1 != pn532.tgInitAsTarget(command, sizeof(command), tgInitAsTargetTimeout)) {
    DMSG("tgInitAsTarget failed or timed out!\r\n");
    return false;
  }

  tagWrittenByInitiator = false;

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

    int16_t rspLen = handleFelicaRequest(rwbuf, (int16_t)status, respbuf, sizeof(respbuf));
    if (rspLen < 0) {
      DMSG("Unsupported or invalid FeliCa request\r\n");
      continue;  // ignore unknown commands, keep emulation alive
    }

    if (!pn532.tgSetData(respbuf, (uint8_t)rspLen)) {
      DMSG("tgSetData failed\r\n");
      pn532.inRelease();
      return true;
    }
  }
}
