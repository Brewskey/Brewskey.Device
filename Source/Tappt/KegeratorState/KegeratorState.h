#ifndef KegeratorState_h
#define KegeratorState_h

#include "DeviceSettings.h"
#include "DeviceStatus.h"
#include "Display.h"
#include "PourDisplay.h"
#include "TotpDisplay.h"
#include "ITick.h"
#include "IStateManager.h"
#include "Tap.h"
#include "LED.h"
#include "Pins.h"
#include "NfcClient.h"
#include "Sensors.h"
#include "ServerLink.h"
#include "TapptTimer.h"

#define TIME_TO_POUR =

class KegeratorState: public ITick, public IStateManager  {
public:
  KegeratorState(Display* display);
  virtual void TapStartedPouring(ITap &tap);
  virtual void TapStoppedPouring(
    ITap &tap,
    uint totalPulses,
    String authenticationKey
  );
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
  void NfcLoop();
  void StopPouring();

  DeviceSettings *settings;
  Display *display;
  PourDisplay *pourDisplay;
  TotpDisplay *totpDisplay;
  NfcClient *nfcClient;
  Sensors *sensors;
  ServerLink *serverLink;
  Tap *taps;

  bool canPourWithoutDeviceId = true;
  int state;
  String oldCode;
  String lastAuthorizedToken;
  int displayChangeCount = 0;

  unsigned long pourResponseStartTime;

  Timer nfcTimer = Timer(1, &KegeratorState::NfcLoop, *this);
};

#endif
