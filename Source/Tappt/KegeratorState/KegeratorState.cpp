#include "KegeratorState.h"

KegeratorState::KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter, LED* led) {
  this->nfcClient = nfcClient;
  this->flowMeter = flowMeter;
  this->led = led;

  // This is to reset the settings on the device via a serverside call
  Particle.function("initialize", &KegeratorState::Initialize, this);
  Particle.function("pour", &KegeratorState::Pour, this);

  // TODO -
  Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);
}

int KegeratorState::Tick()
{
  led->Tick();

  switch(this->State) {
    case KegeratorState::INITIALIZING:
    {
      if (this->deviceId != NULL && this->deviceId.length() > 0) {
        this->led->IsBreathing(false);
        this->State = KegeratorState::LISTENING;
      }

      this->getIdTimer.Tick();

      if (this->getIdTimer.ShouldTrigger) {
        Serial.println("Requesting DeviceId");
        Particle.publish("tappt_initialize", (const char *)0, 5, PRIVATE);
      }

      break;
    }
    case KegeratorState::LISTENING:
    {
      NfcState::value nfcState = (NfcState::value)nfcClient->Tick();

      if ((
        nfcState == NfcState::SENT_MESSAGE ||
        nfcState == NfcState::READ_MESSAGE) &&
        this->State != KegeratorState::POURING
      ) {
        this->State = KegeratorState::WAITING_FOR_POUR_RESPONSE;
        this->responseTimer.Reset();

        this->led->IsBreathing(false);
        this->led->SetColor(255, 255, 0);
      }

      break;
    }

    case KegeratorState::WAITING_FOR_POUR_RESPONSE:
    {
      this->responseTimer.Tick();

      if (this->responseTimer.ShouldTrigger) {
        this->State = KegeratorState::LISTENING;
      }

      break;
    }
    case KegeratorState::POURING:
    {
      int isPouring = flowMeter->Tick();

      if (isPouring <= 0) {
        this->State = KegeratorState::LISTENING;
      }

      break;
    }
  }

  return 0;
}

int KegeratorState::Initialize(String data) {
  this->deviceId = data;
  return this->nfcClient->Initialize(data);
}


int KegeratorState::Pour(String data) {
  this->led->SetColor(0,255,0);
  this->flowMeter->StartPour(data);
  this->State = KegeratorState::POURING;
}
