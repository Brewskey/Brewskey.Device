#ifndef ServerLink_h
#define ServerLink_h

#include "application.h"
#include "DeviceSettings.h"
#include "Tappt/KegeratorState/IStateManager.h"

class ServerLink {
public:
  ServerLink(IStateManager *stateManager);
  void AuthorizePour(uint32_t deviceId, String authenticationKey);
  void SendPourToServer(uint32_t tapId, uint totalPulses, String authenticationKey);

  // public for testing
  void Initialize(const char* event, const char* data);
  void PourResponse(const char* event, const char* data);
private:
  int Pour(String data);
  int Settings(String data);
  void CallInitialize();

  IStateManager *stateManager;

  DeviceSettings settings;
  Timer initializeTimer = Timer(15000, &ServerLink::CallInitialize, *this);
  char json[160];
};

#endif
