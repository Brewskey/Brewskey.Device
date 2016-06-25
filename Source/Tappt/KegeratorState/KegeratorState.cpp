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

	this->displayTimer.start();

	for (int i = 0; i < TAP_COUNT; i++) {
		this->taps[i].Setup(this);
	}
}


uint8_t drawingState = 0;
float iter;
String lastOunces[4];
String currentOunces[4];

void KegeratorState::UpdateScreen() {
	return;
	switch(this->state) {
		case KegeratorState::INITIALIZING: {
			return;
		}

		case KegeratorState::LISTENING: {
			if (drawingState != 11) {
				drawingState = 11;
				this->display->BeginBatch();
				this->display->SetText("...", 42, 35);
				this->display->EndBatch();
			}
			return;
		}

		case KegeratorState::POUR_AUTHORIZED: {
			if (drawingState != 1) {
				drawingState = 1;
				this->display->BeginBatch();
				this->display->SetText("Start", 53, 15);
				this->display->SetText("Pouring", 42, 35);
				this->display->EndBatch();
			}

			return;
		}

		case KegeratorState::INACTIVE: {
			if (drawingState != 2) {
				drawingState = 2;
				this->display->BeginBatch(false);
				this->display->SetText("Device", 28, 15);
				this->display->SetText("Disabled", 16, 35);
				this->display->EndBatch();
			}

			return;
		}

		case KegeratorState::CLEANING: {
			if (drawingState != 3) {
				drawingState = 3;
				this->display->BeginBatch(false);
				this->display->SetText("Cleaning", 16, 15);
				this->display->SetText("Beer Lines", 4, 35);
				this->display->EndBatch();
			}

			return;
		}
	}

	return;

	TOTP totp = TOTP(
		(uint8_t*)this->settings->authorizationToken.c_str(),
		this->settings->authorizationToken.length()
	);
  String newCode = String(totp.getCode((long)Time.now()));

	if (this->oldCode != newCode) {
		Serial.print("TOTP: ");
		Serial.println(newCode.c_str());
	}

	bool totpChanged = this->oldCode != newCode;
	this->oldCode = newCode;


	uint8_t tapPouringCount = 0;
	uint tapsPulses[TAP_COUNT];

/*
	tapPouringCount++;
	tapPouringCount++;
	tapsPulses[0] = (uint)iter;
	iter += 0.1;
	tapsPulses[1] = (uint)iter;
*/
	for (int i = 0; i < TAP_COUNT; i++) {
		if (!this->taps[i].IsPouring()) {
			continue;
		}

		tapPouringCount++;
		tapsPulses[tapPouringCount] = this->taps[i].GetTotalPulses();
	}

	uint8_t totpX = 42;
	uint8_t totpY = 26;
	bool isSingleTap = this->settings->isSingleTap;
	uint8_t showLogo = tapPouringCount < 2;

	// if single - keep logo, replace totp
	// if multiple/single tap - keep logo totp top, pulses underneath
	// if m/2tap - no logo totp top mid/ left + right pours
	// if > 2tap - no logo totp mid mid / corners
	this->display->BeginBatch(showLogo);

	if (tapPouringCount == 1) {
		totpY = 10;

		// Draw single ounces underneath
		this->SetOuncesForPulses(0);
		if (lastOunces[0] != currentOunces[0]) {
			uint8_t y =  isSingleTap ? 26 : 40;
			if (lastOunces[0] != "") {
				//this->display->FillRect(0, 42, lastOunces[0].length() * 12, y);
			}
			this->display->SetText(lastOunces[0], 42, y);
		}
	} else if (tapPouringCount == 2) {
		totpX = 32;
		totpY = 10;

		this->SetOuncesForPulses(0, true);
		if (lastOunces[0] != currentOunces[0]) {
			//this->display->FillRect(0, 40, ounceString.length() * 12, 12);
			this->display->SetText(ounceString, 0, 40);
		}

		if (lastOunces[0] != currentOunces[0]) {
			this->SetOuncesForPulses(1, true);
			//this->display->SetText(ounceString, 128 - length * 12, 40);
		}
	} else if (tapPouringCount == 3){
		totpX = 32;
		totpY = 10;

		this->SetOuncesForPulses(tapsPulses[0], true);
		this->display->SetText(ounceString, 0, 40);

		uint8_t length = String(ounceString).length();
		this->SetOuncesForPulses(tapsPulses[1], true);
		this->display->SetText(ounceString, 128 - length * 12, 40);
	} else {

	}

	if (!isSingleTap && tapPouringCount != 4 && totpChanged) {
		this->display->SetText(newCode, totpX, totpY);
	}

	this->display->EndBatch();
}

void KegeratorState::SetOuncesForPulses(uint8_t tapSlot, bool hideOz) {
	uint pulses = this->taps[tapSlot].GetTotalPulses();
	float ounces = (float)pulses * (float)128 / (float)10313;

	sprintf(
    this->ounceString,
    hideOz ? "%.1f" : "%.1f oz",
    ounces
  );

	currentOunces[tapSlot] = String(this->ounceString);
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
		this->responseTimer.start();
		this->SetState(KegeratorState::WAITING_FOR_POUR_RESPONSE);
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

	if (
		this->settings->deviceId.length() <= 0 ||
		this->settings->authorizationToken.length() <= 0
	) {
		return;
	}

	if (this->settings->deviceStatus == "2") {
		this->SetState(KegeratorState::INACTIVE);
	} else if (this->settings->deviceStatus == "3") {
		this->SetState(KegeratorState::CLEANING);
	} else {
		this->SetState(KegeratorState::LISTENING);
	}

  this->nfcClient->Initialize(this->settings->deviceId);
	this->responseTimer.stopFromISR();
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

	this->responseTimer.startFromISR();
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

	this->responseTimer.stopFromISR();

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
	this->responseTimer.stopFromISR();

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
