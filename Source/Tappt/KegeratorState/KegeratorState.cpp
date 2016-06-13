#include "KegeratorState.h"

#define TAP_COUNT 4

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(
	Display* display
) {
	this->SetState(KegeratorState::INITIALIZING);

	this->display = display;
	this->serverLink = new ServerLink(this);
	this->nfcClient = new NfcClient(this->serverLink);
	this->taps = new Tap[TAP_COUNT];
	this->sensors = new Sensors(this->taps, TAP_COUNT);

	for (int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].Setup(this);
	}
}

void KegeratorState::UpdateScreen() {
	if (this->state != KegeratorState::LISTENING) {
		return;
	}

	TOTP totp = TOTP(
		(uint8_t*)this->settings->authorizationToken.c_str(),
		this->settings->authorizationToken.length()
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

void KegeratorState::SetState(e newState) {
	switch(newState) {
		case KegeratorState::INITIALIZING: {
			RGB.control(true);
	    RGB.color(255, 255, 255);
      break;
    }

		case KegeratorState::LISTENING: {
			RGB.control(false);
			break;
		}

		case KegeratorState::WAITING_FOR_POUR_RESPONSE: {
			RGB.control(true);
			RGB.color(255, 255, 0);
			break;
		}

		case KegeratorState::POUR_AUTHORIZED: {
			RGB.control(true);
			RGB.color(0, 255, 0);
			break;
		}

		case KegeratorState::POURING: {
			RGB.control(true);
			RGB.color(0, 255, 0);
			break;
		}

		case KegeratorState::INACTIVE: {
			RGB.control(true);
			RGB.color(255, 0, 0);

			this->StopPouring();
			break;
		}

		case KegeratorState::CLEANING: {
			RGB.control(true);
			RGB.color(255, 0, 0);

			this->cleaningTimer.reset();
			this->cleaningTimer.start();
			this->sensors->OpenSolenoids();

			break;
		}
	}

	this->state = newState;
}

int KegeratorState::Tick()
{


	NfcState::value nfcState = (NfcState::value)nfcClient->Tick();

	if (
		nfcState == NfcState::SENT_MESSAGE ||
		nfcState == NfcState::READ_MESSAGE
	) {
		this->SetState(KegeratorState::WAITING_FOR_POUR_RESPONSE);
		this->responseTimer.start();
	}

  return 0;
}

void KegeratorState::Initialize(DeviceSettings *settings) {
	this->settings = settings;

	Serial.println("STRTOK");
	char strBuffer[60] = "";
  String(settings->tapIds).toCharArray(strBuffer, 60);
  char* tapId = strtok(strBuffer,",");
	uint iter = 0;

  while (tapId != NULL && iter < TAP_COUNT) {
		this->taps[iter].SetId(String(tapId));
		iter++;
		tapId = strtok (NULL, ",");
  }

	this->StopPouring();


	if (this->settings->deviceStatus == "2") {
		this->SetState(KegeratorState::INACTIVE);

		this->display->BeginBatch(false);
		this->display->SetText("Device", 28, 15);
		this->display->SetText("Disabled", 16, 35);
		this->display->EndBatch();
	} else if (this->settings->deviceStatus == "3") {
		this->SetState(KegeratorState::CLEANING);

		this->display->BeginBatch(false);
		this->display->SetText("Cleaning", 16, 15);
		this->display->SetText("Beer Lines", 4, 35);
		this->display->EndBatch();
	}

	if (
		this->settings->deviceId.length() <= 0 ||
		this->settings->authorizationToken.length() <= 0
	) {
		return;
	}

	this->SetState(KegeratorState::LISTENING);
  this->nfcClient->Initialize(this->settings->deviceId);

	this->ledTimer.start();
	this->responseTimer.stop();
}

int KegeratorState::Settings(String data) {
	this->SetState(KegeratorState::INITIALIZING);

	return 0;
}

int KegeratorState::StartPour(String data) {
	this->lastAuthorizedToken = data;

	// This is only really necessary when anonymous pours are turned off
	// When anon pours are enabled, the solenoid isn't used.
	this->sensors->OpenSolenoids();

	this->SetState(KegeratorState::POUR_AUTHORIZED);
	this->responseTimer.reset();

	return 0;
}

void KegeratorState::CleaningComplete() {
	this->SetState(KegeratorState::INACTIVE);

	this->display->BeginBatch(false);
	this->display->SetText("Device", 28, 15);
	this->display->SetText("Disabled", 16, 35);
	this->display->EndBatch();
}

void KegeratorState::TapStartedPouring(ITap &tap) {
	this->SetState(KegeratorState::POURING);
	Serial.println("Started Pouring");

	this->responseTimer.stop();

	tap.SetAuthToken(this->lastAuthorizedToken);

	this->lastAuthorizedToken = "";
	this->CleanupTapState();
}

void KegeratorState::TapStoppedPouring(
	ITap &tap,
	uint totalPulses,
	String authenticationKey
) {
	Serial.println("Finished Pouring");

	if (totalPulses > PULSE_EPSILON) {
		this->serverLink->SendPourToServer(
			tap.GetId(),
			totalPulses,
			authenticationKey
		);

		// TODO - Maybe show pulse on display a bit longer...?
	}

	this->CleanupTapState();
}

void KegeratorState::StopPouring() {
	for(int i = 0; i < TAP_COUNT; i++) {
		if (this->taps[i].IsPouring()) {
			this->taps[i].StopPour();
		}
	}
}


void KegeratorState::Timeout() {
	this->lastAuthorizedToken = "";

	this->CleanupTapState();
}

void KegeratorState::CleanupTapState() {
	bool allStopped = true;
	for(int i = 0; i < TAP_COUNT; i++) {
		if (this->taps[i].IsPouring()) {
			allStopped = false;
		} else {
			this->sensors->CloseSolenoid(i);
		}
	}

	if (allStopped) {
		this->SetState(KegeratorState::LISTENING);
	}
}
