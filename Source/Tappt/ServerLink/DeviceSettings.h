#ifndef DeviceSettings_h
#define DeviceSettings_h

#include "application.h"

struct DeviceSettings {
  String authorizationToken;
  String deviceId;
  String deviceStatus;
  bool isSingleTap;
  String tapIds;
};

#endif
