#include "KegeratorState.h"

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(
	NfcClient* nfcClient,
	FlowMeter* flowMeter,
	Solenoid* solenoid,
	Display* display
) {
	this->display = display;
  this->nfcClient = nfcClient;
  this->flowMeter = flowMeter;

	String deviceID = System.deviceID();

  Particle.subscribe(
		"hook-response/tappt_initialize-" + deviceID,
		&KegeratorState::Initialize,
		this,
		MY_DEVICES
	);

	// Called by server when device settings are updated.
	Particle.function("settings", &KegeratorState::Settings, this);

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
}

void KegeratorState::UpdateScreen() {
	if (this->State != KegeratorState::LISTENING) {
		return;
	}

	TOTP totp = TOTP(
		(uint8_t*)this->authorizationToken.c_str(),
		this->authorizationToken.length()
	);
  String newCode = String(totp.getCode((long)Time.now()));

	if (this->oldCode == newCode) {
		return;
	}

	this->oldCode = newCode;

	WITH_LOCK(Serial) {
		Serial.print("TOTP: ");
		Serial.println(newCode.c_str());
	}

	this->display->BeginBatch();
	this->display->SetText(newCode, 42, 26);
	this->display->EndBatch();
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

		case KegeratorState::CLEANING:
		case KegeratorState::INACTIVE: {
			// Don't do anything
			break;
		}
  }

  return 0;
}

void KegeratorState::Initialize(const char* event, const char* data) {
  if (strlen(data) <= 0) {
    return;
  }

	char strBuffer[90] = "";
  String(data).toCharArray(strBuffer, 90);

  this->deviceId = String(strtok(strBuffer, "~"));
	this->authorizationToken = String(strtok((char*)NULL, "~"));
	String tapIds = String(strtok((char*)NULL, "~"));
	String deviceStatus = String(strtok((char*)NULL, "~"));

	Serial.print("Tap IDs: ");
	Serial.println(tapIds);

	Serial.print("Device Status: ");
	Serial.println(deviceStatus);

	this->solenoid->Close();

	if (deviceStatus == "2") {
		this->State = KegeratorState::INACTIVE;

		this->display->BeginBatch(false);
		this->display->SetText("Device", 28, 15);
		this->display->SetText("Disabled", 16, 35);
		this->display->EndBatch();

		RGB.control(true);
		RGB.color(255, 0, 0);

	} else if (deviceStatus == "3") {
		this->State = KegeratorState::CLEANING;

		this->cleaningTimer.reset();
		this->cleaningTimer.start();

		this->display->BeginBatch(false);
		this->display->SetText("Cleaning", 16, 15);
		this->display->SetText("Beer Lines", 4, 35);
		this->display->EndBatch();

		this->solenoid->Open();

		RGB.control(true);
		RGB.color(255, 0, 0);
	}

	if (this->deviceId.length() <= 0 || this->authorizationToken.length() <= 0) {
		return;
	}

  this->nfcClient->Initialize(this->deviceId);

	this->ledTimer.start();
}

int KegeratorState::Settings(String data) {
	Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);
	this->State = KegeratorState::INITIALIZING;

	return 0;
}

int KegeratorState::Pour(String data) {
	Serial.print("Pour: ");
	Serial.println(data);

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

void KegeratorState::CleaningComplete() {
	this->State = KegeratorState::INACTIVE;

	this->display->BeginBatch(false);
	this->display->SetText("Device", 28, 15);
	this->display->SetText("Disabled", 16, 35);
	this->display->EndBatch();

	this->solenoid->Close();
}
