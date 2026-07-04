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

bool EmulateTag::emulateStart() {
  if (armed) {
    return true;
  }

  // http://www.nxp.com/documents/application_note/AN133910.pdf
  uint8_t command[] = {
      PN532_COMMAND_TGINITASTARGET,
      targetMode, // MODE: 0x05 = PICC only + Passive only, 0x00 = unrestricted

      sensRes[0], sensRes[1], // SENS_RES
      0x00, 0x00, 0x00,       // NFCID1
      selRes,                 // SEL_RES

      // FeliCaParams. The chip's AutoColl state answers FeliCa (NFC-F)
      // polling no matter what, and phones (Samsung especially) poll NFC-F
      // aggressively — some never give the Type A side a turn while an F
      // response exists. So instead of hiding, the F side is a real NFC
      // Forum Type 3 Tag: IDm prefix 0x02 0xFE (T3T platform), NDEF system
      // code 0x12FC, and handleFelicaTransaction() serves the same NDEF
      // message over FeliCa Check commands.
      felicaIdm[0], felicaIdm[1], felicaIdm[2], felicaIdm[3], // NFCID2
      felicaIdm[4], felicaIdm[5], felicaIdm[6], felicaIdm[7],
      felicaPmm[0], felicaPmm[1], felicaPmm[2], felicaPmm[3], // PAD / PMm
      felicaPmm[4], felicaPmm[5], felicaPmm[6], felicaPmm[7],
      0x12, 0xFC,                                             // SystemCode

      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // NFCID3t

      0, // length of general bytes
      0  // length of historical bytes
  };

  if (uidPtr != 0) {  // if uid is set copy 3 bytes to nfcid1
    memcpy(command + 4, uidPtr, 3);
  }

  if (0 != pn532.tgInitAsTargetStart(command, sizeof(command))) {
    DMSG("tgInitAsTargetStart failed!\r\n");
    // The chip may have accepted the frame even though the ACK read failed;
    // abort so host and chip agree no command is pending.
    pn532.abortCommand();
    return false;
  }

  armed = true;
  return true;
}

int8_t EmulateTag::emulatePoll() {
  if (!armed) {
    return EMULATE_POLL_FAILED;
  }

  int8_t status = pn532.tgInitAsTargetPoll();

  if (status == 0) {
    return EMULATE_POLL_WAITING;
  }

  // Either activated or errored; the command is no longer pending.
  armed = false;

  if (status < 0) {
    // The response was unreadable (corrupt/partial frame). Abort so neither
    // a stale command nor a queued response survives into the next arm.
    pn532.abortCommand();
    return EMULATE_POLL_FAILED;
  }

  // Mode byte: bits 1..0 framing (00 = Mifare, 10 = FeliCa), bit 2 DEP,
  // bit 3 ISO14443-4 PICC, bits 5..4 baudrate (00 = 106k).
  uint8_t activationMode = pn532.getLastTargetActivationMode();
  uint8_t framing = activationMode & 0x03;

  // If the chip completed TgInitAsTarget on frame reception, the
  // initiator's first frame is in the response and will never arrive via
  // TgGetData/TgGetInitiatorCommand; hand it to the transaction handler.
  uint8_t firstFrame[64];
  uint8_t firstFrameLength =
      pn532.getTargetFirstFrame(firstFrame, sizeof(firstFrame));

  // FeliCa (non-DEP) activation: serve the NDEF message as an NFC Forum
  // Type 3 Tag.
  if (framing == 0x02 && (activationMode & 0x04) == 0) {
    return handleFelicaTransaction(firstFrame, firstFrameLength)
               ? EMULATE_POLL_MESSAGE_READ
               : EMULATE_POLL_FAILED;
  }

  // Anything else that is not an ISO-DEP PICC activation (NFC-DEP, active
  // mode, ISO14443-3 without RATS) is released; we cannot serve it.
  if (framing != 0x00 || (activationMode & 0x08) == 0) {
    DMSG("Unsupported activation, releasing\r\n");
    pn532.inRelease();
    return EMULATE_POLL_FAILED;
  }

  return handleTransaction(firstFrame, firstFrameLength)
             ? EMULATE_POLL_MESSAGE_READ
             : EMULATE_POLL_FAILED;
}

int8_t EmulateTag::emulateAbort() {
  if (!armed) {
    return EMULATE_POLL_WAITING;
  }

  // An initiator may have activated us since the last poll; aborting now
  // would drop the phone mid-transaction (it already got our ATS and is
  // waiting for APDU responses). Check once and serve it instead.
  int8_t status = emulatePoll();
  if (status != EMULATE_POLL_WAITING) {
    // emulatePoll disarmed us and either handled the transaction or cleaned
    // up the error; nothing is pending anymore.
    return status;
  }

  pn532.abortCommand();
  // If activation raced the abort, a completed TgInitAsTarget response may
  // still be queued and could survive the ACK; drain it so the next command
  // doesn't read it as its own reply.
  pn532.tgInitAsTargetPoll();
  armed = false;
  return EMULATE_POLL_WAITING;
}

bool EmulateTag::handleTransaction(const uint8_t* firstCommand,
                                   uint8_t firstCommandLength) {
  uint8_t compatibility_container[] = {
    0, 0x0F,
    0x20,
    0, 0x54,
    0, 0x7B,  // MLc: must stay within rwbuf (128) minus APDU overhead
    0x04,       // T
    0x06,       // L
    0xE1, 0x04, // File identifier
    ((NDEF_MAX_LENGTH & 0xFF00) >> 8), (NDEF_MAX_LENGTH & 0xFF), // maximum NDEF file size
    0x00,       // read access 0x0 = granted
    0x00        // write access 0x0 = granted | 0xFF = deny
  };

  if (tagWriteable == false) {
    compatibility_container[14] = 0xFF;
  }

  tagWrittenByInitiator = false;

  uint8_t rwbuf[128];
  uint8_t sendlen = 0;
  int16_t status;
  tag_file currentFile = NONE_Tag_File;
  uint16_t cc_size = sizeof(compatibility_container);
  bool messageRead = false;
  uint32_t transactionStart = millis();
  uint16_t apduCount = 0;
  // A C-APDU is at least CLA INS P1 P2 Lc; anything shorter in the
  // activation response (RATS, PPS) was already handled by the chip.
  bool haveFirstCommand = firstCommand != 0 && firstCommandLength >= 5 &&
                          firstCommandLength <= sizeof(rwbuf);

  while (true) {
    if ((millis() - transactionStart) > EMULATE_MAX_TRANSACTION_MS ||
        ++apduCount > EMULATE_MAX_TRANSACTION_APDUS) {
      DMSG("transaction cap hit - bailing\r\n");
      break;
    }

    if (haveFirstCommand) {
      memcpy(rwbuf, firstCommand, firstCommandLength);
      status = firstCommandLength;
      haveFirstCommand = false;
    } else {
      status = pn532.tgGetData(rwbuf, sizeof(rwbuf));

      if (status < 0) {
        // Initiator left the field or deselected us; this is the normal way
        // a transaction ends.
        DMSG("tgGetData failed!\r\n");
        break;
      }
    }

    // Too short to be a C-APDU (CLA INS P1 P2 Lc); don't parse garbage
    // beyond the received bytes. Seen in the field as stray 1-byte frames.
    if (status < 5) {
      setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
      if (!pn532.tgSetData(rwbuf, sendlen)) {
        break;
      }
      continue;
    }

    uint8_t p1 = rwbuf[C_APDU_P1];
    uint8_t p2 = rwbuf[C_APDU_P2];
    uint8_t lc = rwbuf[C_APDU_LC];
    uint16_t p1p2_length = ((int16_t)p1 << 8) + p2;
    bool responseHasNdefData = false;

    // The response is built in rwbuf: keep Le within it (2 bytes reserved
    // for the status word). Our CC advertises MLe = 0x54 so compliant
    // readers never ask for more anyway.
    if (lc > sizeof(rwbuf) - 2) {
      lc = sizeof(rwbuf) - 2;
    }

    switch (rwbuf[C_APDU_INS]) {
    case ISO7816_SELECT_FILE:
      switch (p1) {
      case C_APDU_P1_SELECT_BY_ID:
        if (p2 != 0x0c) {
          DMSG("C_APDU_P2 != 0x0c\r\n");
          setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
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
        const uint8_t ndef_tag_application_name_v2[] = { 0, 0x7, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
        if (0 == memcmp(ndef_tag_application_name_v2, rwbuf + C_APDU_P2, sizeof(ndef_tag_application_name_v2))) {
          setResponse(COMMAND_COMPLETE, rwbuf, &sendlen);
        }
        else {
          DMSG("function not supported\r\n");
          setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
        }
        break;
      }
      default:
        // Unknown P1: without this, sendlen stays uninitialized and
        // tgSetData sends garbage.
        setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
        break;
      }
      break;
    case ISO7816_READ_BINARY:
      switch (currentFile) {
      case NONE_Tag_File:
        setResponse(TAG_NOT_FOUND, rwbuf, &sendlen);
        break;
      case CC_Tag_File:
        if (p1p2_length + lc > cc_size) {
          setResponse(END_OF_FILE_BEFORE_REACHED_LE_BYTES, rwbuf, &sendlen);
        } else {
          memcpy(rwbuf, compatibility_container + p1p2_length, lc);
          setResponse(COMMAND_COMPLETE, rwbuf + lc, &sendlen, lc);
        }
        break;
      case NDEF_Tag_File:
        if (p1p2_length + lc > NDEF_MAX_LENGTH) {
          setResponse(END_OF_FILE_BEFORE_REACHED_LE_BYTES, rwbuf, &sendlen);
        } else {
          memcpy(rwbuf, ndef_file + p1p2_length, lc);
          setResponse(COMMAND_COMPLETE, rwbuf + lc, &sendlen, lc);
          // Reading past the 2-byte NLEN prefix means this response carries
          // actual NDEF message bytes, not just the length header. Only
          // latched below once tgSetData confirms the response was sent.
          if (p1p2_length + lc > 2) {
            responseHasNdefData = true;
          }
        }
        break;
      }

      break;
    case ISO7816_UPDATE_BINARY:
      if (!tagWriteable) {
        setResponse(FUNCTION_NOT_SUPPORTED, rwbuf, &sendlen);
      }
      else {
        // The incoming data starts at C_APDU_DATA, so the copy below reads
        // rwbuf[C_APDU_DATA .. C_APDU_DATA+lc-1] — bound lc to the buffer.
        if (p1p2_length + lc > NDEF_MAX_LENGTH ||
            lc > sizeof(rwbuf) - C_APDU_DATA) {
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
      DMSG("tgSetData failed\r\n!");
      break;
    }

    // Only count the message as delivered once the response actually went
    // out; latching earlier would report SENT_MESSAGE for a phone that left
    // the field before receiving the NDEF bytes.
    if (responseHasNdefData) {
      messageRead = true;
    }
  }

  DMSG("Emulation finished\r\n");
  // A host-side timeout above (tgGetData/tgSetData) can leave the command
  // pending inside the chip; abort it so inRelease and the next arm start
  // from an idle chip. Harmless when nothing is pending.
  pn532.abortCommand();
  pn532.inRelease();
  return messageRead;
}

// ---- FeliCa / NFC Forum Type 3 Tag emulation ----
//
// Served when a phone activates us with FeliCa framing instead of
// ISO14443A. Frames are exchanged with their leading LEN byte, matching
// the FeliCa air format.

#define T3T_CMD_POLLING             0x00
#define T3T_CMD_REQUEST_SERVICE     0x02
#define T3T_CMD_REQUEST_RESPONSE    0x04
#define T3T_CMD_CHECK               0x06
#define T3T_CMD_UPDATE              0x08
#define T3T_CMD_REQUEST_SYSTEM_CODE 0x0C

// Our 64-byte frame budget allows LEN+CMD+IDm+2 status+count+3*16 data.
#define T3T_MAX_BLOCKS_PER_READ 3

void EmulateTag::buildT3tBlock(uint16_t blockNumber, uint8_t* out) {
  memset(out, 0, 16);
  uint16_t ndefLength = (ndef_file[0] << 8) + ndef_file[1];

  if (blockNumber == 0) {
    // Attribute information block (T3T spec 1.0)
    out[0] = 0x10;                     // Ver 1.0
    out[1] = T3T_MAX_BLOCKS_PER_READ;  // Nbr: max blocks per Check
    out[2] = 0x01;                     // Nbw (tag is read-only anyway)
    uint16_t nmaxb = (NDEF_MAX_LENGTH - 2 + 15) / 16;
    out[3] = nmaxb >> 8;
    out[4] = nmaxb & 0xFF;
    // out[5..8] unused
    out[9] = 0x00;                     // WriteF: no write in progress
    out[10] = 0x00;                    // RW flag: read only
    out[11] = 0x00;                    // Ln (3 bytes, big endian)
    out[12] = ndefLength >> 8;
    out[13] = ndefLength & 0xFF;
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 14; i++) {
      sum += out[i];
    }
    out[14] = sum >> 8;
    out[15] = sum & 0xFF;
    return;
  }

  // Data blocks carry the NDEF message, 16 bytes each, zero padded.
  uint16_t offset = (uint16_t)(blockNumber - 1) * 16;
  for (uint8_t i = 0; i < 16; i++) {
    if (offset + i < ndefLength) {
      out[i] = ndef_file[2 + offset + i];
    }
  }
}

uint8_t EmulateTag::buildT3tResponse(const uint8_t* frame, uint8_t frameLength,
                                     uint8_t* response,
                                     bool* servedNdefData) {
  *servedNdefData = false;
  if (frameLength < 2) {
    return 0;
  }

  uint8_t cmd = frame[1];

  if (cmd == T3T_CMD_POLLING) {
    // LEN 00 SC0 SC1 RC TSN
    if (frameLength < 6) {
      return 0;
    }
    uint16_t systemCode = ((uint16_t)frame[2] << 8) | frame[3];
    if (systemCode != 0xFFFF && systemCode != 0x12FC) {
      return 0;  // not for us
    }
    uint8_t requestCode = frame[4];
    uint8_t len = (requestCode != 0) ? 20 : 18;
    response[0] = len;
    response[1] = 0x01;
    memcpy(response + 2, felicaIdm, 8);
    memcpy(response + 10, felicaPmm, 8);
    if (requestCode != 0) {
      response[18] = 0x12;
      response[19] = 0xFC;
    }
    return len;
  }

  // All other commands carry the IDm at bytes 2..9; ignore frames addressed
  // to someone else.
  if (frameLength < 10 || 0 != memcmp(frame + 2, felicaIdm, 8)) {
    return 0;
  }

  switch (cmd) {
    case T3T_CMD_REQUEST_SERVICE: {
      uint8_t count = frame[10];
      if (count == 0 || count > 12 || frameLength < 11 + 2 * count) {
        return 0;
      }
      uint8_t len = 11 + 2 * count;
      response[0] = len;
      response[1] = 0x03;
      memcpy(response + 2, felicaIdm, 8);
      response[10] = count;
      for (uint8_t i = 0; i < count; i++) {
        // Service codes are little endian; only the read-only NDEF service
        // 0x000B exists on this tag.
        bool exists = frame[11 + 2 * i] == 0x0B && frame[12 + 2 * i] == 0x00;
        response[11 + 2 * i] = exists ? 0x00 : 0xFF;
        response[12 + 2 * i] = exists ? 0x00 : 0xFF;
      }
      return len;
    }

    case T3T_CMD_REQUEST_RESPONSE: {
      response[0] = 11;
      response[1] = 0x05;
      memcpy(response + 2, felicaIdm, 8);
      response[10] = 0x00;  // mode
      return 11;
    }

    case T3T_CMD_REQUEST_SYSTEM_CODE: {
      response[0] = 13;
      response[1] = 0x0D;
      memcpy(response + 2, felicaIdm, 8);
      response[10] = 0x01;
      response[11] = 0x12;
      response[12] = 0xFC;
      return 13;
    }

    case T3T_CMD_CHECK: {
      // LEN 06 IDm8 m services(2m) n blocklist
      if (frameLength < 12) {
        return 0;
      }
      uint8_t serviceCount = frame[10];
      uint8_t index = 11 + 2 * serviceCount;
      if (serviceCount == 0 || frameLength < index + 1) {
        return 0;
      }
      uint8_t blockCount = frame[index++];

      uint8_t status1 = 0x00;
      uint8_t status2 = 0x00;
      uint16_t blocks[T3T_MAX_BLOCKS_PER_READ];

      if (blockCount == 0 || blockCount > T3T_MAX_BLOCKS_PER_READ) {
        status1 = 0x01;
        status2 = 0xA2;  // block count out of range
      } else {
        uint16_t nmaxb = (NDEF_MAX_LENGTH - 2 + 15) / 16;
        for (uint8_t i = 0; i < blockCount && status1 == 0; i++) {
          if (frameLength < index + 2) {
            return 0;
          }
          uint8_t element = frame[index];
          uint16_t blockNumber;
          if (element & 0x80) {  // 2-byte block list element
            blockNumber = frame[index + 1];
            index += 2;
          } else {  // 3-byte element, block number little endian
            if (frameLength < index + 3) {
              return 0;
            }
            blockNumber = frame[index + 1] | ((uint16_t)frame[index + 2] << 8);
            index += 3;
          }
          if (blockNumber > nmaxb) {
            status1 = 0x01;
            status2 = 0xA2;
          } else {
            blocks[i] = blockNumber;
          }
        }
      }

      if (status1 != 0) {
        response[0] = 12;
        response[1] = 0x07;
        memcpy(response + 2, felicaIdm, 8);
        response[10] = status1;
        response[11] = status2;
        return 12;
      }

      uint8_t len = 13 + 16 * blockCount;
      response[0] = len;
      response[1] = 0x07;
      memcpy(response + 2, felicaIdm, 8);
      response[10] = 0x00;
      response[11] = 0x00;
      response[12] = blockCount;
      for (uint8_t i = 0; i < blockCount; i++) {
        buildT3tBlock(blocks[i], response + 13 + 16 * i);
        if (blocks[i] >= 1) {
          // NDEF message content (not just the attribute block) was read.
          *servedNdefData = true;
        }
      }
      return len;
    }

    case T3T_CMD_UPDATE: {
      response[0] = 12;
      response[1] = 0x09;
      memcpy(response + 2, felicaIdm, 8);
      response[10] = 0x01;
      response[11] = 0xA6;  // write not permitted: read-only tag
      return 12;
    }

    default:
      return 0;
  }
}

bool EmulateTag::handleFelicaTransaction(const uint8_t* firstFrame,
                                         uint8_t firstFrameLength) {
  DMSG("FeliCa/T3T session\r\n");
  bool messageRead = false;
  uint32_t transactionStart = millis();
  uint16_t frameCount = 0;

  // Normalize the first frame into felicaFrame with its LEN byte at [0].
  uint8_t frameLength = 0;
  if (firstFrame != 0 && firstFrameLength >= 2) {
    if (firstFrame[0] == firstFrameLength &&
        firstFrameLength <= sizeof(felicaFrame)) {
      memcpy(felicaFrame, firstFrame, firstFrameLength);
      frameLength = firstFrameLength;
    } else if (firstFrameLength < sizeof(felicaFrame)) {
      // The chip delivered the frame without its LEN byte; restore it.
      felicaFrame[0] = firstFrameLength + 1;
      memcpy(felicaFrame + 1, firstFrame, firstFrameLength);
      frameLength = firstFrameLength + 1;
    }
  }

  while (true) {
    if ((millis() - transactionStart) > EMULATE_MAX_TRANSACTION_MS ||
        ++frameCount > EMULATE_MAX_TRANSACTION_APDUS) {
      DMSG("T3T transaction cap hit\r\n");
      break;
    }

    if (frameLength >= 2) {
      DMSG("T3T cmd:");
      for (uint8_t i = 0; i < frameLength; i++) {
        DMSG_HEX(felicaFrame[i]);
      }
      DMSG("\r\n");

      bool servedNdefData = false;
      uint8_t responseLength = buildT3tResponse(
          felicaFrame, frameLength, felicaResponse, &servedNdefData);

      if (responseLength > 0) {
        if (!pn532.tgResponseToInitiator(felicaResponse, responseLength)) {
          DMSG("tgResponseToInitiator failed\r\n");
          break;
        }
        if (servedNdefData) {
          messageRead = true;
        }
      }
    }

    int16_t status =
        pn532.tgGetInitiatorCommand(felicaFrame, sizeof(felicaFrame), 100);
    if (status <= 0) {
      DMSG("T3T session ended\r\n");
      break;
    }
    frameLength = (uint8_t)status;
    // Normalize: the chip may or may not include the LEN byte.
    if (felicaFrame[0] != frameLength) {
      if (frameLength >= sizeof(felicaFrame)) {
        break;
      }
      memmove(felicaFrame + 1, felicaFrame, frameLength);
      felicaFrame[0] = frameLength + 1;
      frameLength++;
    }
  }

  // Leave the chip idle for the next arm regardless of how the session
  // ended.
  pn532.abortCommand();
  pn532.inRelease();
  return messageRead;
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
