#ifndef DeviceStatus_h
#define DeviceStatus_h

#include "application.h"

class DeviceStatus {
public:
  enum e {
    FREE = 0,
    ACTIVE = 1,
    INACTIVE = 2,
    CLEANING = 3,
  };
};

#endif
