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
  Particle.function("pour", &KegeratorState::Pour, this);

  //Particle.subscribe("hook-response/", &sunTimeHandler, MY_DEVICES);
  Particle.subscribe("hook-response/tappt_initialize", &KegeratorState::Initialized, this, MY_DEVICES);// System.deviceID());
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
  int resultCode;
  jsmn_parser p;
  jsmntok_t tokens[10]; // a number >= total number of tokens

  jsmn_init(&p);
  resultCode = jsmn_parse(&p, data, tokens, 10);

  if (resultCode < 0) {
    Serial.println("Bad initialize value.");
    return;
  }

  if (!TOKEN_STRING(data, tokens[1], "deviceId")) {
    Serial.println("Initialize - no device ID");
    return;
  }

	Serial.println(data);
	Serial.print("start ");
	Serial.println(tokens[2].start);
	Serial.print("size ");
	Serial.println(tokens[2].size);

  char deviceId[12];
  memcpy(deviceId, &data[tokens[2].start], tokens[2].start - tokens[2].end);
  deviceId[11] = '\0';

	Serial.println(String(deviceId));
	return;
  this->deviceId = String(deviceId);
  this->nfcClient->Initialize(String(data));
}


int KegeratorState::Pour(String data) {
  this->led->SetColor(0,255,0);
  this->flowMeter->StartPour(data);
  this->State = KegeratorState::POURING;
}
