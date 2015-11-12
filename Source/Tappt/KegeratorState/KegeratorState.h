#ifndef KegeratorState_h
#define KegeratorState_h

#include "FlowMeter.h"
#include "NfcClient.h"

class KegeratorState: public ITick  {
public:
  KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter);
  virtual int Tick();
  
  int State = KegeratorState::LISTENING;

  enum e {
    LISTENING,
    WAITING_FOR_POUR_RESPONSE,
    POURING,
    DONE_POURING,
  };
//private:
  void SetNfcState(NfcState::value nfcState);

  NfcClient* nfcClient;
  FlowMeter* flowMeter;
};

#endif
