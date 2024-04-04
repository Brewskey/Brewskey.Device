#ifndef DeviceNFCStatus_h
#define DeviceNFCStatus_h

#include "application.h"

class DeviceNFCStatus {
 public:
  enum e {
    PHONE_ONLY = 0,
    CARD_ONLY = 1,
    PHONE_AND_CARD = 2,
    DISABLED = 3,
  };
};

#endif
