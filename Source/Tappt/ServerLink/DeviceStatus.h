#ifndef DeviceStatus_h
#define DeviceStatus_h

#include "application.h"

class DeviceStatus {
public:
  enum e {
    ACTIVE = 1,
    INACTIVE = 2,
    CLEANING = 3,
    UNLOCKED = 4,
    CONFIGURE = 5,
  };
};

#endif
