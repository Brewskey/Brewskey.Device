#ifndef KegeratorState_h
#define KegeratorState_h

#include "Tappt/ServerLink/DeviceSettings.h"
#include "Tappt/ServerLink/DeviceStatus.h"
#include "Tappt/Display/Display.h"
#include "Tappt/Display/PourDisplay.h"
#include "Tappt/Display/TotpDisplay.h"
#include "Tappt/ITick.h"
#include "Tappt/KegeratorState/IStateManager.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/led/LED.h"
#include "Tappt/Pins.h"
#include "Tappt/NfcClient/NfcClient.h"
#include "Tappt/Sensors/Sensors.h"
#include "Tappt/ServerLink/ServerLink.h"
#include "Tappt/TapptTimer/TapptTimer.h"

//#define TIME_TO_POUR =
#define MILLISECONDS_IN_HOUR 10000

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
    FREE_POUR,
    INACTIVE,
  };

private:
  void SetState(e state);
  void Timeout();
  void CleanupTapState();
  void NfcLoop();
  void StopPouring();

  void StartCleaning();
  void StartInactive();

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

  // Add extra delay so the server can switch this over instead of us
  TapptTimer openValveTimer = TapptTimer(3000, MILLISECONDS_IN_HOUR + 30000);
};

#endif
