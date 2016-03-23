#include "KegeratorState.h"

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(
	NfcClient* nfcClient,
	FlowMeter* flowMeter,
	Display* display
) {
	this->display = display;
  this->nfcClient = nfcClient;
  this->flowMeter = flowMeter;

	String deviceID = System.deviceID();

  Particle.subscribe(
		"hook-response/tappt_initialize-" + deviceID,
		&KegeratorState::Initialized,
		this,
		MY_DEVICES
	);

	// Called by server when user tries to pour.
	Particle.function("pour", &KegeratorState::Pour, this);
	// Response when token is used
	Particle.subscribe(
		"hook-response/tappt_request-pour-" + deviceID,
		&KegeratorState::PourResponse,
		this,
		MY_DEVICES
	);

  Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);

	//initecc(3, 10);
	//initframe();
}

void KegeratorState::UpdateScreen() {
	WITH_LOCK(Serial) {
		Serial.print("Auth Token: ");
		Serial.println(this->authorizationToken);

		TOTP totp = TOTP(
			(uint8_t*)this->authorizationToken.c_str(),
			this->authorizationToken.length()
		);
	  String newCode = String(totp.getCode((long)Time.now()));

		if (this->oldCode == newCode) {
			return;
		}

		this->oldCode = newCode;

		Serial.print("TOTP: ");
		Serial.println(newCode.c_str());

		this->display->UpdateTOTP(newCode);

		/*
		char json[20];
		sprintf(
			json,
			"%s:%s",
			newCode.c_str(),
			this->deviceId.c_str()
		);

		strcpy((char *)strinbuf, json);

    qrencode();

		Serial.print("WIDTH: ");
		Serial.println(WD);
		Serial.println();

		this->display->UpdateQR();
		*/
  }
}

int KegeratorState::Tick()
{
  switch(this->State) {
    case KegeratorState::INITIALIZING:
    {
      if (this->deviceId != NULL && this->deviceId.length() > 0) {
        RGB.control(false);
				this->State = KegeratorState::LISTENING;
      }

      this->getIdTimer.Tick();

      if (this->getIdTimer.ShouldTrigger) {
        Serial.println("Requesting DeviceId");
        Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);
      }

      break;
    }
    case KegeratorState::LISTENING:
    {
			this->flowMeter->StopPour();

      NfcState::value nfcState = (NfcState::value)nfcClient->Tick();

      if ((
        nfcState == NfcState::SENT_MESSAGE ||
        nfcState == NfcState::READ_MESSAGE) &&
        this->State != KegeratorState::POURING
      ) {
        this->State = KegeratorState::WAITING_FOR_POUR_RESPONSE;
        this->responseTimer.Reset();

				RGB.control(true);
        RGB.color(255, 255, 0);
      }

      break;
    }

    case KegeratorState::WAITING_FOR_POUR_RESPONSE:
    {
      this->responseTimer.Tick();

      if (this->responseTimer.ShouldTrigger) {
				RGB.control(false);
        this->State = KegeratorState::LISTENING;
      }

      break;
    }
    case KegeratorState::POURING:
    {
      int isPouring = flowMeter->Tick();

      if (isPouring <= 0) {
        RGB.control(false);
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

	String str = String(data);
  char strBuffer[64] = "";
  str.toCharArray(strBuffer, 64);

  this->deviceId = String(strtok(strBuffer, "~"));
	this->authorizationToken = String(strtok((char*)NULL, "~"));
  this->nfcClient->Initialize(this->deviceId);

	this->ledTimer.start();
}

int KegeratorState::Pour(String data) {
	if (data.length() <= 0) {
		return -1;
	}

	RGB.control(true);
	RGB.color(0, 255, 0);
	this->State = KegeratorState::POURING;

	this->flowMeter->StartPour(data);
	return 0;
}

void KegeratorState::PourResponse(const char* event, const char* data) {
	this->Pour(String(data));
}
