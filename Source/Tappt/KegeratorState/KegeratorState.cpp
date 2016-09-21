#include "KegeratorState.h"

#define TAP_COUNT 1

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(
	Display* display,
	NfcClient* nfcClient
) {
	this->SetState(KegeratorState::INITIALIZING);

	this->display = display;

	this->serverLink = new ServerLink(this);
	nfcClient->Setup(this->serverLink);
	this->nfcClient = nfcClient;

	this->taps = new Tap[TAP_COUNT];
	for (int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].Setup(this);
	}

	this->sensors = new Sensors(this->taps, TAP_COUNT);
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

			// TODO - only unlock for an hour
			this->sensors->OpenSolenoids();

			break;
		}
	}

	this->state = newState;
}

int KegeratorState::Tick()
{
  // While initializing nothing is set up :(
  if (this->state == KegeratorState::INITIALIZING) {
    return 0;
  }

  unsigned long pourResponseDelta = millis() - this->pourResponseStartTime;

  // Only check this if waiting for the response and it's greater that 5
  // seconds
  if (
    this->state == KegeratorState::WAITING_FOR_POUR_RESPONSE &&
    pourResponseDelta > 5000
  ) {
    this->Timeout();
  }

  // If the pour is authorized, you have 5 seconds to pour
  if (
    this->state == KegeratorState::POUR_AUTHORIZED &&
    pourResponseDelta > 5000
  ) {
    this->Timeout();
  }

  this->sensors->Tick();

  // read taps and manage end-pour
  for(int i = 0; i < TAP_COUNT; i++) {
    this->taps[i].Tick();

    // Anonymous pour started
    if (
      this->state == KegeratorState::LISTENING &&
      this->taps[i].IsPouring()
    ) {
      this->SetState(KegeratorState::POURING);
    }
	}
  return 0;
}

void KegeratorState::NfcLoop() {
	NfcState::value nfcState = (NfcState::value)nfcClient->Tick();

	if (
		nfcState == NfcState::SENT_MESSAGE ||
		nfcState == NfcState::READ_MESSAGE
	) {
    this->pourResponseStartTime = millis();
		this->SetState(KegeratorState::WAITING_FOR_POUR_RESPONSE);
	}
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

	if (
		this->settings->deviceId.length() <= 0 ||
		this->settings->authorizationToken.length() <= 0
	) {
		return;
	}

  this->nfcTimer.stop();
	if (this->settings->deviceStatus == "2") {
		this->SetState(KegeratorState::INACTIVE);
	} else if (this->settings->deviceStatus == "3") {
		this->SetState(KegeratorState::CLEANING);
	} else {
		this->SetState(KegeratorState::LISTENING);
    this->nfcTimer.start();
	}

  this->nfcClient->Initialize(this->settings->deviceId);
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
  this->pourResponseStartTime = millis();
	this->SetState(KegeratorState::POUR_AUTHORIZED);

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

	if (this->lastAuthorizedToken != NULL && this->lastAuthorizedToken.length()) {
		tap.SetAuthToken(this->lastAuthorizedToken);
	}

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
      // TODO - Send half-pour to server.  We don't want to forget to track this
			this->taps[i].StopPour();
		}
	}
}


void KegeratorState::Timeout() {
	this->lastAuthorizedToken = "";

	Serial.println("");
	Serial.println("TIMEOUT");
	Serial.println("");

	this->CleanupTapState();
}

void KegeratorState::CleanupTapState() {
	this->oldCode = "";

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
