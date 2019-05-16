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

  // How bright the blue LEDs are
  uint8_t ledBrightness = 255;
  bool isTOTPDisabled = false;
  bool isScreenDisabled = false;

  // The before pour and after pour time the valve should stay open.
  uint8_t timeForValveOpen = 5;
  // Used when going into cleaning or free-pour mode
  unsigned long secondsToStayOpen = 3600;
};

#endif
