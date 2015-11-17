#include "KegeratorState.h"

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

void sunTimeHandler(const char * event, const char * data)
{
  Serial.println("Initialized");
  Serial.println(event);
  Serial.println(data);
  Serial.println(System.deviceID());
  Serial.println(".");
}

KegeratorState::KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter, LED* led) {
  this->nfcClient = nfcClient;
  this->flowMeter = flowMeter;
  this->led = led;

  // This is to reset the settings on the device via a serverside call
  //Particle.function("initialize", &KegeratorState::Initialize, this);
  //Particle.function("pour", &KegeratorState::Pour, this);

  Particle.subscribe(
		"hook-response/tappt_initialize-" + System.deviceID(),
		&KegeratorState::Initialized,
		this,
		MY_DEVICES
	);

	Particle.subscribe(
		"hook-response/tappt_request-pour-" + System.deviceID(),
		&KegeratorState::Pour,
		this,
		MY_DEVICES
	);

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

void KegeratorState::Initialized(const char* event, const char* data) {
  if (strlen(data) <= 0) {
    return;
  }

  this->deviceId = String(data);
  this->nfcClient->Initialize(this->deviceId);
}


void KegeratorState::Pour(const char* event, const char* data) {
	if (strlen(data) <= 0) {
    return;
  }

  this->led->SetColor(0,255,0);
  this->flowMeter->StartPour(String(data));
  this->State = KegeratorState::POURING;
}
