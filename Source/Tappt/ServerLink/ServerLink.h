#ifndef ServerLink_h
#define ServerLink_h

#include "application.h"
#include "DeviceSettings.h"
#include "IStateManager.h"

class ServerLink {
public:
  ServerLink(IStateManager *stateManager);
private:
  void InitializeComplete(const char* event, const char* data);
  int Pour(String data);
  void PourResponse(const char* event, const char* data);
  int Settings(String data);
  void CallInitialize();

  IStateManager *stateManager;

  DeviceSettings settings;
  Timer initializeTimer = Timer(15000, &ServerLink::CallInitialize, *this);
};

#endif
