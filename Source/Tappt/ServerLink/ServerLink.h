#ifndef ServerLink_h
#define ServerLink_h

#include "application.h"
#include "DeviceSettings.h"
#include "IStateManager.h"

class ServerLink {
public:
  ServerLink(IStateManager *stateManager);
  void AuthorizePour(String deviceId, String authenticationKey);
  void SendPourToServer(String tapId, uint totalPulses, String authenticationKey);
private:
  void Initialize(const char* event, const char* data);
  int Pour(String data);
  void PourResponse(const char* event, const char* data);
  int Settings(String data);
  void CallInitialize();

  IStateManager *stateManager;

  DeviceSettings settings;
  Timer initializeTimer = Timer(15000, &ServerLink::CallInitialize, *this);
  char json[160];
};

#endif
