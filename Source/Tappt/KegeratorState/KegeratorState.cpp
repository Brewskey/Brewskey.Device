#include "KegeratorState.h"

KegeratorState::KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter) {
  this->nfcClient = nfcClient;
  this->flowMeter = flowMeter;
}

int KegeratorState::Tick()
{
  return 0;
}

void KegeratorState::SetNfcState(NfcState::value nfcState) {
  if ((
    nfcState == NfcState::SENT_MESSAGE ||
    nfcState == NfcState::READ_MESSAGE) &&
    this->State != KegeratorState::POURING
  ) {
    this->State = KegeratorState::WAITING_FOR_POUR_RESPONSE;
  }
}
