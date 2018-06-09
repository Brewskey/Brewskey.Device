#ifndef ServerLink_h
#define ServerLink_h

#include "application.h"
#include "Tappt/Pins.h"
#include "DeviceSettings.h"
#include "Tappt/KegeratorStateMachine/IKegeratorStateMachine.h"

class ServerLink {
public:
  ServerLink(IKegeratorStateMachine *kegeratorStateMachine);
  void AuthorizePour(uint32_t deviceId, String authenticationKey);
  void SendPourToServer(uint32_t tapId, uint32_t totalPulses, String authenticationKey);

  // public for testing
  void Initialize(const char* event, const char* data);
  void PourResponse(const char* event, const char* data);
private:
  int Pour(String data);
  int Settings(String data);
  void CallInitialize();

  IKegeratorStateMachine *kegeratorStateMachine;

  DeviceSettings settings;
  Timer initializeTimer = Timer(15000, &ServerLink::CallInitialize, *this);
  char json[160];
};

#endif
