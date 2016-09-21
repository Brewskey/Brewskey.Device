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
  KegeratorState(Display* display, NfcClient* nfcClient);
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
  void NfcLoop();

  //void UpdateScreen();
  //void SetOuncesForPulses(uint8_t tapSlot, bool hideOz = false);
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
  char ounceString[7];

  unsigned long pourResponseStartTime;

  // TODO - get rid of as many of these as
  /*
  Timer displayTimer = Timer(100, &KegeratorState::UpdateScreen, *this);
  Timer cleaningTimer = Timer(60000 * 60, &KegeratorState::CleaningComplete, *this, true);
  Timer responseTimer = Timer(5000, &KegeratorState::Timeout, *this, true);
  */

  Timer nfcTimer = Timer(1, &KegeratorState::NfcLoop, *this);

  TapptTimer getIdTimer = TapptTimer(15000);
};

#endif
