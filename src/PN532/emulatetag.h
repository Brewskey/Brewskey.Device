/**************************************************************************/
/*!
    @file     emulatetag.h
    @author   Armin Wieser
    @license  BSD

    Implemented using NFC forum documents & library of libnfc
*/
/**************************************************************************/

#ifndef __EMULATETAG_H__
#define __EMULATETAG_H__

#include "PN532/PN532.h"

#define NDEF_MAX_LENGTH 256  // altough ndef can handle up to 0xfffe in size, arduino cannot.

// Guards against a hostile/broken initiator keeping the APDU loop alive
// forever (it runs on the shared software-timer thread). A normal read
// transaction is 4-6 APDUs and completes in well under 500ms.
#define EMULATE_MAX_TRANSACTION_MS 1500
#define EMULATE_MAX_TRANSACTION_APDUS 64

typedef enum { COMMAND_COMPLETE, TAG_NOT_FOUND, FUNCTION_NOT_SUPPORTED, MEMORY_FAILURE, END_OF_FILE_BEFORE_REACHED_LE_BYTES } responseCommand;

// Result of emulatePoll()
#define EMULATE_POLL_WAITING 0     // target armed, no initiator yet
#define EMULATE_POLL_MESSAGE_READ 1 // an initiator read the NDEF message
#define EMULATE_POLL_FAILED (-1)   // transaction failed or ended without a read; re-arm

class EmulateTag {

public:
  EmulateTag(PN532Interface &interface_) : pn532(interface_), uidPtr(0), tagWrittenByInitiator(false), tagWriteable(true), updateNdefCallback(0), armed(false) {
    targetMode = 0x05;
    sensRes[0] = 0x04;
    sensRes[1] = 0x00;
    selRes = 0x20;
  }

  bool init();

  /*
   * Non-blocking emulation. emulateStart() arms the chip as a passive
   * ISO14443-4 target and returns immediately; the chip stays armed (no RF
   * field of its own) until an initiator activates it. Poll with
   * emulatePoll(); once an initiator activates us the full APDU transaction
   * is handled inside that call. emulateAbort() cancels a pending arm so the
   * chip can be used as a reader.
   */
  bool emulateStart();
  int8_t emulatePoll();
  // Cancels a pending arm. If an initiator activated us in the instant
  // before the abort, the transaction is served instead of dropped and the
  // emulatePoll() result is returned (EMULATE_POLL_MESSAGE_READ /
  // EMULATE_POLL_FAILED); returns EMULATE_POLL_WAITING when the arm was
  // idle and has been aborted cleanly.
  int8_t emulateAbort();
  bool isArmed() { return armed; }

  /*
   * TgInitAsTarget activation parameters: MODE byte (0x05 = PICC-only +
   * passive-only, 0x00 = unrestricted) and the ISO14443A SENS_RES/ATQA and
   * SEL_RES/SAK bytes. Exposed so different combinations can be A/B tested
   * against phone NFC stacks.
   */
  void setTargetParams(uint8_t mode, uint8_t sensRes0, uint8_t sensRes1,
                       uint8_t selRes_) {
    targetMode = mode;
    sensRes[0] = sensRes0;
    sensRes[1] = sensRes1;
    selRes = selRes_;
  }

  /*
   * @param uid pointer to byte array of length 3 (uid is 4 bytes - first byte is fixed) or zero for uid
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

  uint16_t getNdefMaxLength() {
    return NDEF_MAX_LENGTH;
  }

  void attach(void(*func)(uint8_t *buf, uint16_t length)) {
    updateNdefCallback = func;
  };

private:
  bool handleTransaction(const uint8_t* firstCommand,
                         uint8_t firstCommandLength);
  bool handleFelicaTransaction(const uint8_t* firstFrame,
                               uint8_t firstFrameLength);
  uint8_t buildT3tResponse(const uint8_t* frame, uint8_t frameLength,
                           uint8_t* response, bool* servedNdefData);
  void buildT3tBlock(uint16_t blockNumber, uint8_t* out);

  PN532 pn532;
  uint8_t ndef_file[NDEF_MAX_LENGTH];
  uint8_t* uidPtr;
  bool tagWrittenByInitiator;
  bool tagWriteable;
  void(*updateNdefCallback)(uint8_t *ndef, uint16_t length);
  bool armed;
  uint8_t targetMode;
  uint8_t sensRes[2];
  uint8_t selRes;
  // FeliCa / NFC Forum Type 3 Tag identity. NFCID2 (IDm) prefix 0x02 0xFE
  // marks a Type 3 Tag platform (0x01 0xFE would mean NFC-DEP). System
  // code 0x12FC is the NDEF system code Android/iOS look for.
  uint8_t felicaIdm[8] = {0x02, 0xFE, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5};
  uint8_t felicaPmm[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  // Session buffers are members: the NFC timer thread stack is small.
  uint8_t felicaFrame[64];
  uint8_t felicaResponse[64];

  void setResponse(responseCommand cmd, uint8_t* buf, uint8_t* sendlen, uint8_t sendlenOffset = 0);
};

#endif
