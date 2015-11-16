#ifndef KegeratorState_h
#define KegeratorState_h

#include "jsmn.h"
#include "FlowMeter.h"
#include "LED.h"
#include "NfcClient.h"
#include "TapptTimer.h"

class KegeratorState: public ITick  {
public:
  KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter, LED* led);
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

  NfcClient* nfcClient;
  FlowMeter* flowMeter;
  LED* led;

  String deviceId;
  TapptTimer getIdTimer = TapptTimer(10000);
  TapptTimer responseTimer = TapptTimer(3000);
};

#endif
