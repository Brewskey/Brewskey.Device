#ifndef DeviceSettings_h
#define DeviceSettings_h

#include "application.h"

struct DeviceSettings {
  String authorizationToken;
  String deviceId;
  uint8_t deviceStatus;
  uint32_t  *tapIds = NULL;
  uint8_t tapCount;
  uint32_t *pulsesPerGallon = NULL;
};

#endif
