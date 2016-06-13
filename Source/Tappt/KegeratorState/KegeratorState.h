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

#define TIME_TO_POUR =

class KegeratorState: public ITick, public IStateManager  {
public:
  KegeratorState(Display* display);
  virtual void TapStartedPouring(ITap &tap);
  virtual void TapStoppedPouring(ITap &tap, uint totalPulses, String authenticationKey);
  virtual int Tick();
  void Initialize(DeviceSettings *settings);
  int StartPour(String data);
  int Settings(String data);

  int GetState();

  enum e {
    INITIALIZING,
    LISTENING,
    WAITING_FOR_POUR_RESPONSE,
    POUR_AUTHORIZED,
    POURING,

    CLEANING,
    INACTIVE,
  };

private:
  void SetState(e state);
  void CleaningComplete();
  void Timeout();
  void CleanupTapState();

  void UpdateScreen();
  void StopPouring();

  DeviceSettings *settings;
  Display *display;
  NfcClient *nfcClient;
  Sensors *sensors;
  ServerLink *serverLink;
  Tap *taps;

  bool canPourWithoutDeviceId = true;
  int state;
  String oldCode;
  String lastAuthorizedToken;

  // TODO - get rid of as many of these as
  Timer ledTimer = Timer(1000, &KegeratorState::UpdateScreen, *this);
  Timer cleaningTimer = Timer(60000 * 60, &KegeratorState::CleaningComplete, *this, true);
  Timer responseTimer = Timer(5000, &KegeratorState::Timeout, *this, true);
  TapptTimer getIdTimer = TapptTimer(15000);
};

#endif
