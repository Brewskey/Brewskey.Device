/**************************************************************************/
/*!
    @file     emulatetag.h
    @author   Armin Wieser
    @license  BSD

    NFC Forum Type 3 Tag (FeliCa/NFCF) emulation only.
    Implemented using NFC Forum Type 3 Tag 1.1 and PN532 as target.
*/
/**************************************************************************/

#ifndef __EMULATETAG_H__
#define __EMULATETAG_H__

#include "PN532/PN532.h"

#define NDEF_MAX_LENGTH 256  // although ndef can handle up to 0xfffe in size, arduino cannot.

// FeliCa command codes
#define FELICA_CMD_REQUEST_SERVICE    0x02
#define FELICA_CMD_READ_WITHOUT_ENC   0x06
#define FELICA_CMD_WRITE_WITHOUT_ENC  0x08
// FeliCa response codes (command + 1)
#define FELICA_RSP_REQUEST_SERVICE    0x03
#define FELICA_RSP_READ_WITHOUT_ENC   0x07
#define FELICA_RSP_WRITE_WITHOUT_ENC  0x09

// NFC Forum Type 3 Tag (FeliCa) service codes, LSB first in frames
#define TYPE3_SERVICE_NDEF_READ   0x090F  // bytes: 0x0F, 0x09
#define TYPE3_SERVICE_NDEF_WRITE  0x090B  // bytes: 0x0B, 0x09
#define TYPE3_SYSTEM_CODE         0x88B4  // bytes: 0xB4, 0x88
#define FELICA_BLOCK_SIZE          16

// TgGetData timeout for FeliCa (ms); readers may be slower than ISO14443
#define FELICA_TGGETDATA_TIMEOUT_MS 400

class EmulateTag {

public:
  EmulateTag(PN532Interface &interface_) : pn532(interface_), uidPtr(0), tagWrittenByInitiator(false), tagWriteable(true), updateNdefCallback(0), currentFile(0) { }

  bool init();

  bool emulate(const uint16_t tgInitAsTargetTimeout = 0);

  /*
   * For NFCF: optional pointer to 8 bytes used as NFCID2t (IDm) suffix.
   * NFCID2t is 8 bytes and must start with 01 FE; we use 01 FE + first 6 bytes of uid.
   * If zero, a default NFCID2t is used.
   */
  void setUid(uint8_t* uid = 0);

  void setNdefFile(const uint8_t* ndef, const int16_t ndefLength);

  void getContent(uint8_t** buf, uint16_t* length) {
    *buf = ndef_file + 2; // first 2 bytes = length
    *length = (ndef_file[0] << 8) + ndef_file[1];
  }

  bool writeOccured() {
    return tagWrittenByInitiator;
  }

  void setTagWriteable(bool setWriteable) {
    tagWriteable = setWriteable;
  }

  uint8_t* getNdefFilePtr() {
    return ndef_file;
  }

  uint8_t getNdefMaxLength() {
    return NDEF_MAX_LENGTH;
  }

  void attach(void(*func)(uint8_t *buf, uint16_t length)) {
    updateNdefCallback = func;
  };

private:
  PN532 pn532;
  uint8_t ndef_file[NDEF_MAX_LENGTH];
  uint8_t* uidPtr;
  bool tagWrittenByInitiator;
  bool tagWriteable;
  void(*updateNdefCallback)(uint8_t *ndef, uint16_t length);
  uint8_t currentFile;  // Type 4: NONE_Tag_File, CC_Tag_File, NDEF_Tag_File

  void buildAttributeBlock(uint8_t* out);
  void getBlock(uint16_t blockNo, uint8_t* out);
  int16_t handleFelicaRequest(uint8_t* req, int16_t reqLen, uint8_t* resp, uint8_t respMaxLen);
  int16_t handleType4Apdu(uint8_t* req, int16_t reqLen, uint8_t* resp, uint8_t respMaxLen);
};

#endif
