#ifndef __TAG_INFORMATION_H__
#define __TAG_INFORMATION_H__

#include "application.h"

class TagInformation {
public:
  uint16_t sens_res;
  uint16_t sak;
  uint8_t uidLength;
  uint8_t *uid;

  enum type {
    UNKNOWN,
    DESFIRE_EV1,
    DESFIRE_EV1_RANDOM,
    MIFARE_CLASSIC,
    MIFARE_ULTRALIGHT,
  };

  TagInformation::type GetTagType();
};

#endif
