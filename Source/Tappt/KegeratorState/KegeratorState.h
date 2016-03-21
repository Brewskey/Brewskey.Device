#ifndef KegeratorState_h
#define KegeratorState_h

#include "FlowMeter.h"
#include "LED.h"
#include "NfcClient.h"
#include "TapptTimer.h"
#include "TOTP.h"
#include "qrencode.h"

class KegeratorState: public ITick  {
public:
  KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter);
  virtual int Tick();

  int State = KegeratorState::INITIALIZING;

  enum e {
    INITIALIZING,
    LISTENING,
    WAITING_FOR_POUR_RESPONSE,
    POURING,

    // TODO server should be able to put the hardware into this state for 20
    // minutes.  A timer will close the valve.
    CLEANING,
  };

private:
  void Initialized(const char* event, const char* data);
  int Pour(String data);
  void PourResponse(const char* event, const char* data);
  void UpdateScreen();

  NfcClient* nfcClient;
  FlowMeter* flowMeter;

  String authorizationToken;
  String deviceId;
  Timer ledTimer = Timer(10000, &KegeratorState::UpdateScreen, *this);
  TapptTimer getIdTimer = TapptTimer(15000);
  TapptTimer responseTimer = TapptTimer(3000);
};

#endif
