#include "KegeratorState.h"

#define TAP_COUNT 4

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(
	NfcClient* nfcClient,
	Display* display
) {
	this->display = display;
	this->nfcClient = nfcClient;
	this->taps = new Tap[TAP_COUNT];
	this->sensors = new Sensors(this->taps, TAP_COUNT);
	this->serverLink = new ServerLink(this);

	for (int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].Setup(this);
	}
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
			for (uint8_t i = 0; i < TAP_COUNT; i++) {
				if (this->taps[i].IsPouring()) {
					break;
				}
			}

      RGB.control(false);
			this->State = KegeratorState::LISTENING;

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

void KegeratorState::Initialize(DeviceSettings *settings) {
	this->settings = settings;

	for(int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].StopPour();
	}

	if (this->settings->deviceStatus == "2") {
		this->State = KegeratorState::INACTIVE;

		this->display->BeginBatch(false);
		this->display->SetText("Device", 28, 15);
		this->display->SetText("Disabled", 16, 35);
		this->display->EndBatch();

		RGB.control(true);
		RGB.color(255, 0, 0);

	} else if (this->settings->deviceStatus == "3") {
		this->State = KegeratorState::CLEANING;

		this->cleaningTimer.reset();
		this->cleaningTimer.start();

		this->display->BeginBatch(false);
		this->display->SetText("Cleaning", 16, 15);
		this->display->SetText("Beer Lines", 4, 35);
		this->display->EndBatch();

		for(int i = 0; i < TAP_COUNT; i++) {
			this->taps[i].OpenValve();
		}

		RGB.control(true);
		RGB.color(255, 0, 0);
	}

	if (
		this->settings->deviceId.length() <= 0 ||
		this->settings->authorizationToken.length() <= 0
	) {
		return;
	}

  this->nfcClient->Initialize(this->settings->deviceId);

	this->ledTimer.start();
}

int KegeratorState::Settings(String data) {
	this->State = KegeratorState::INITIALIZING;

	return 0;
}

int KegeratorState::Pour(String data) {
	RGB.control(true);
	RGB.color(0, 255, 0);
	this->State = KegeratorState::POURING;

	for(int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].OpenValve();
	}

	return 0;
}

void KegeratorState::CleaningComplete() {
	this->State = KegeratorState::INACTIVE;

	this->display->BeginBatch(false);
	this->display->SetText("Device", 28, 15);
	this->display->SetText("Disabled", 16, 35);
	this->display->EndBatch();

	for(int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].StopPour();
	}
}

void KegeratorState::TapIsPouring(ITap &tap) {
	for(int i = 0; i < TAP_COUNT; i++) {
		if (!this->taps[i].IsPouring()) {
			this->taps[i].StopPour();
		}
	}
}
