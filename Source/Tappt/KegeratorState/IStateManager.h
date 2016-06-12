#ifndef IStateManager_h
#define IStateManager_h

#include "DeviceSettings.h"
#include "ITap.h"
#include "application.h"

class IStateManager {
public:
  virtual void TapIsPouring(ITap &tap) = 0;
  void Initialize(DeviceSettings *settings);
  int Pour(String data);
  int Settings(String data);
  virtual ~IStateManager() {}
};

#endif
