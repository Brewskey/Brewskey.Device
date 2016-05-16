#ifndef KegeratorState_h
#define KegeratorState_h

#include "Display.h"
#include "FlowMeter.h"
#include "Solenoid.h"
#include "LED.h"
#include "NfcClient.h"
#include "TapptTimer.h"
#include "TOTP.h"
//#include "qrencode.h"

class KegeratorState: public ITick  {
public:
  KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter, Solenoid* solenoid, Display* display);
  virtual int Tick();

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
  void Initialize(const char* event, const char* data);
  int Pour(String data);
  void PourResponse(const char* event, const char* data);
  int Settings(String data);
  void UpdateScreen();

  Display* display;
  NfcClient* nfcClient;
  FlowMeter* flowMeter;
  Solenoid* solenoid;

  String authorizationToken;
  String deviceId;
  String oldCode;
  Timer ledTimer = Timer(1000, &KegeratorState::UpdateScreen, *this);
  Timer cleaningTimer = Timer(60000 * 60, &KegeratorState::CleaningComplete, *this, true);
  TapptTimer getIdTimer = TapptTimer(15000);
  TapptTimer responseTimer = TapptTimer(3000);
};

#endif
