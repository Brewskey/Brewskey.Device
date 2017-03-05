#ifndef DeviceSettings_h
#define DeviceSettings_h

#include "application.h"

struct DeviceSettings {
  String authorizationToken;
  String deviceId;
  uint8_t deviceStatus;
  String *tapIds;
  uint8_t tapCount;
  int pulsesPerGallon[4];
};

#endif
