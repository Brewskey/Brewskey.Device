#include "KegeratorState.h"

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(
	Display* display,
	NfcClient* nfcClient,
  Sensors* sensors
) {
	this->serverLink = new ServerLink(this);

	this->taps = (Tap*)NULL;
	this->SetState(KegeratorState::INITIALIZING);

	this->display = display;
	this->pourDisplay = new PourDisplay(display);
	this->totpDisplay = new TotpDisplay(display);

  this->nfcClient = nfcClient;
  this->sensors = sensors;
	nfcClient->Setup(this->serverLink);
}

void KegeratorState::SetState(e newState) {
	switch(newState) {
		case KegeratorState::INITIALIZING: {
			RGB.control(true);
	    RGB.color(255, 255, 255);
      break;
    }

		case KegeratorState::LISTENING: {
			Serial.println("LISTENING");
			RGB.control(false);
			break;
		}

		case KegeratorState::WAITING_FOR_POUR_RESPONSE: {
			this->nfcClient->SendPendingMessage();
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

		case KegeratorState::UNLOCKED: {
			RGB.control(true);
			RGB.color(0, 255, 127);

			// If the box is already in free pour mode we don't want to add additional
			// time.
			if (!this->openValveTimer.IsRunning())
			{
				this->openValveTimer.Start();
			}

			break;
		}

		case KegeratorState::INACTIVE: {
			RGB.control(true);
			RGB.color(255, 0, 0);
			this->display->BeginBatch();
			this->display->SetText("Device", 28, 15);
			this->display->SetText("Disabled", 16, 35);
			this->displayChangeCount++;

			this->StopPouring();
			break;
		}

		case KegeratorState::CLEANING: {
			RGB.control(true);
			RGB.color(255, 0, 0);
			this->display->BeginBatch();
			this->display->SetText("Cleaning", 16, 15);
			this->display->SetText("Device", 28, 35);
			this->displayChangeCount++;

			this->openValveTimer.Start();

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

	if (this->openValveTimer.IsRunning())
	{
		this->openValveTimer.Tick();
		if (this->openValveTimer.ShouldTrigger())
		{
			this->sensors->OpenSolenoids();
		}

		if (!this->openValveTimer.IsRunning())
		{
			if (this->state == KegeratorState::CLEANING)
			{
				this->SetState(KegeratorState::INACTIVE);
			}
			else
			{
				this->SetState(KegeratorState::LISTENING);
			}
		}
	}

	// We don't need to run the rest of the rendering logic
	switch (this->state)
	{
		case KegeratorState::CLEANING:
		case KegeratorState::INACTIVE: {
			return 0;
		}
	}

  long pourResponseDelta = millis() - this->pourResponseStartTime;

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

	// read taps and manage end-pour
	for(int i = 0; i < this->settings->tapCount; i++) {
		this->taps[i].Tick();
	}

	// This must come after the taps check otherwise it can result in
	// duplicate pours.
	this->sensors->Tick();


  // Rendering
  this->displayChangeCount += this->pourDisplay->Tick();
	this->displayChangeCount += this->totpDisplay->Tick();

  return 0;
}

void KegeratorState::NfcLoop() {
	// We end the rendering on this loop so it doesn't mess with the NFC
	if (this->displayChangeCount > 0) {
		this->displayChangeCount = 0;
		this->display->EndBatch();
	}

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

	// Setup taps
	if (this->taps != NULL) {
		delete[] this->taps;
	}

	int tapCount = this->settings->tapCount;
	this->taps = new Tap[tapCount];
	for (int i = 0; i < tapCount; i++) {
		this->taps[i].Setup(
			this,
			settings->tapIds[i],
			settings->pulsesPerGallon[i]
		);
	}

	this->sensors->Setup(this->taps, tapCount);
	this->pourDisplay->Setup(this->taps, tapCount);
	this->totpDisplay->Setup(this->settings, this->taps, tapCount);

	this->StopPouring();

	if (
		this->settings->deviceId.length() <= 0 ||
		this->settings->authorizationToken.length() <= 0
	) {
		RGB.control(true);
    RGB.color(0, 128, 0);
		return;
	}

	this->display->BeginBatch();
	this->display->EndBatch();
	this->totpDisplay->Reset();

  this->nfcTimer.stop();
	this->nfcTimer.start();

	this->openValveTimer.Stop();
	this->SetStateFromDeviceStatus();
	this->sensors->CloseSolenoids();

  this->nfcClient->Initialize(this->settings->deviceId);
}

void KegeratorState::SetStateFromDeviceStatus() {
	if (this->settings->deviceStatus == DeviceStatus::INACTIVE) {
		this->SetState(KegeratorState::INACTIVE);
	} else if (this->settings->deviceStatus == DeviceStatus::CLEANING) {
		this->SetState(KegeratorState::CLEANING);
	} else if (this->settings->deviceStatus == DeviceStatus::UNLOCKED) {
		this->SetState(KegeratorState::UNLOCKED);
	} else {
		this->SetState(KegeratorState::LISTENING);
	}
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

void KegeratorState::TapStartedPouring(ITap &tap) {
	this->SetState(KegeratorState::POURING);
	Serial.println("Started Pouring");

	if (this->lastAuthorizedToken != NULL && this->lastAuthorizedToken.length()) {
		tap.SetAuthToken(this->lastAuthorizedToken);
	}

	this->lastAuthorizedToken = "";
}

void KegeratorState::TapStoppedPouring(
	uint32_t tapID,
	uint32_t totalPulses,
	String authenticationKey
) {
	this->CleanupTapState();
	Serial.println("Finished Pouring");

	if (totalPulses > PULSE_EPSILON) {
		this->serverLink->SendPourToServer(
			tapID,
			totalPulses,
			authenticationKey
		);

		// TODO - Maybe show pulse on display a bit longer...?
	}

}

void KegeratorState::StopPouring() {
	for(int i = 0; i < this->settings->tapCount; i++) {
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
	this->oldCode = "";

	bool allStopped = true;
	for(uint8_t ii = 0; ii < this->settings->tapCount; ii++) {
		if (this->taps[ii].IsPouring()) {
			allStopped = false;
		} else {
			this->sensors->CloseSolenoid(ii);
			this->sensors->ResetFlowSensor(ii);
		}
	}

	if (allStopped) {
		this->SetStateFromDeviceStatus();
	}
}
