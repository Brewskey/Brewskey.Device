#pragma once

#include "Tappt/ServerLink/DeviceSettings.h"
#include "Tappt/ServerLink/DeviceStatus.h"
#include "Tappt/Display/Display.h"
#include "Tappt/Display/PourDisplay.h"
#include "Tappt/Display/TotpDisplay.h"
#include "Tappt/KegeratorStateMachine/IKegeratorStateMachine.h"
#include "Tappt/KegeratorStateMachine/KegeratorState.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/led/LED.h"
#include "Tappt/Pins.h"
#include "Tappt/NfcClient/NfcClient.h"
#include "Tappt/Sensors/Sensors.h"
#include "Tappt/ServerLink/ServerLink.h"
#include "Tappt/TapptTimer/TapptTimer.h"

//#define TIME_TO_POUR =
#define MILLISECONDS_IN_HOUR 3600000

class KegeratorStateMachine : public IKegeratorStateMachine {
public:
  KegeratorStateMachine(Display* display, NfcClient* client, Sensors* sensors);
  virtual void TapStartedPouring(ITap &tap);
  virtual void TapStoppedPouring(
    uint32_t tapID,
    uint32_t totalPulses,
    String authenticationKey
  );
  virtual int Tick();
  void Initialize(DeviceSettings *settings);
  int StartPour(String data);
  int Settings(String data);

  void OnConfigureNextBox(uint8_t destination);
private:
  void SetState(KegeratorState::e state);
  void Timeout();
  void CleanupTapState();
  void NfcLoop();
  void StopPouring();

  void StartCleaning();
  void StartInactive();

  void SetStateFromDeviceStatus();

  DeviceSettings *settings = NULL;
  Display *display = NULL;
  PourDisplay *pourDisplay = NULL;
  TotpDisplay *totpDisplay = NULL;
  NfcClient *nfcClient = NULL;
  Sensors *sensors = NULL;
  ServerLink *serverLink = NULL;
  Tap *taps = NULL;

  bool canPourWithoutDeviceId = true;
  KegeratorState::e state;
  String oldCode;
  String lastAuthorizedToken;
  int displayChangeCount = 0;

  unsigned long pourResponseStartTime;

  Timer nfcTimer = Timer(1, &KegeratorStateMachine::NfcLoop, *this);

  // Add extra delay so the server can switch this over instead of us
  TapptTimer openValveTimer = TapptTimer(3000, MILLISECONDS_IN_HOUR + 30000);
};
