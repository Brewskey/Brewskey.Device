#ifndef KegeratorState_h
#define KegeratorState_h

#include "DeviceSettings.h"
#include "Display.h"
#include "ITick.h"
#include "IStateManager.h"
#include "Tap.h"
#include "LED.h"
#include "NfcClient.h"
#include "Sensors.h"
#include "ServerLink.h"
#include "TapptTimer.h"
#include "TOTP.h"

class KegeratorState: public ITick, public IStateManager  {
public:
  KegeratorState(NfcClient* nfcClient, Display* display);
  virtual void TapIsPouring(ITap &tap);
  virtual int Tick();
  void Initialize(DeviceSettings *settings);
  int Pour(String data);
  int Settings(String data);

  int State = KegeratorState::INITIALIZING;

  enum e {
    INITIALIZING,
    LISTENING,
    WAITING_FOR_POUR_RESPONSE,
    POURING,

    CLEANING,
    INACTIVE,
  };

private:
  void CleaningComplete();

  void UpdateScreen();

  DeviceSettings *settings;
  Display *display;
  NfcClient *nfcClient;
  Sensors *sensors;
  ServerLink *serverLink;
  Tap *taps;

  bool canPourWithoutDeviceId = true;
  String authorizationToken;
  String deviceId;
  String oldCode;
  Timer ledTimer = Timer(1000, &KegeratorState::UpdateScreen, *this);
  Timer cleaningTimer = Timer(60000 * 60, &KegeratorState::CleaningComplete, *this, true);
  TapptTimer getIdTimer = TapptTimer(15000);
  TapptTimer responseTimer = TapptTimer(3000);
};

#endif
