#include "KegeratorStateMachine.h"

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorStateMachine::KegeratorStateMachine(
  Display* display,
  NfcClient* nfcClient,
  Sensors* sensors
) {
  this->sensors = sensors;
  this->SetState(KegeratorState::INITIALIZING);

  this->display = display;
  this->pourDisplay = new PourDisplay(display);
  this->totpDisplay = new TotpDisplay(display);

	this->serverLink = new ServerLink(this);

	this->nfcClient = nfcClient;
  nfcClient->Setup(this->serverLink);
}

void KegeratorStateMachine::SetState(KegeratorState::e newState) {
  switch (newState) {
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
			if (
				this->state != KegeratorState::POUR_AUTHORIZED &&
				this->state != KegeratorState::POURING
			) {
		    RGB.control(true);
		    RGB.color(255, 255, 0);
			}
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

			this->StopPouring();
	    this->openValveTimer.Start();

	    break;
	  }

		case KegeratorState::CONFIGURE: {
	    RGB.control(true);
	    RGB.color(255, 255, 0);
	    this->OnConfigureNextBox(0);

			this->StopPouring();

	    this->openValveTimer.Start();

	    break;
	  }
  }

  this->state = newState;
	this->sensors->SetState(newState);
}

void KegeratorStateMachine::OnConfigureNextBox(uint8_t destination)
{
	this->display->BeginBatch();
	this->displayChangeCount++;

	uint8_t tapIndex = destination * MAX_TAP_COUNT_PER_BOX + 1;
	if (tapIndex > this->settings->tapCount) {
		this->settings->deviceStatus = DeviceStatus::ACTIVE;
		this->StopPouring();
		this->SetStateFromDeviceStatus();
		return;
	}

	this->display->SetText("Pour From", 16, 15);
	this->display->SetText("Tap " + String(tapIndex), 28, 35);
}

int KegeratorStateMachine::Tick()
{
  // While initializing nothing is set up :(
  if (this->state == KegeratorState::INITIALIZING) {
    return 0;
  }

	this->openValveTimer.Tick();
  if (this->openValveTimer.IsRunning())
  {
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
	  case KegeratorState::CLEANING: {
			this->sensors->Tick();

			return 0;
		}
	  case KegeratorState::INACTIVE: {
	    return 0;
	  }
  }

	if (this->state == KegeratorState::CONFIGURE) {
		this->sensors->Tick();
		return 0;
	}

  long pourResponseDelta = millis() - this->pourResponseStartTime;

  // Only check this if waiting for the response and it's greater that 10
  // seconds. This authorization is coming from the server so we don't need to
	// make it configurable
  if (
    this->state == KegeratorState::WAITING_FOR_POUR_RESPONSE &&
    pourResponseDelta > 5000
  ) {
    this->Timeout();
  }

  // If the pour is authorized, you have n seconds to pour
  if (
    this->state == KegeratorState::POUR_AUTHORIZED &&
    pourResponseDelta > this->settings->timeForValveOpen * 1000
  ) {
    this->Timeout();
  }

  // read taps and manage end-pour
  for (int i = 0; i < this->settings->tapCount; i++) {
    this->taps[i].Tick();
  }

  // This must come after the taps check otherwise it can result in
  // duplicate pours.
  this->sensors->Tick();

  // Rendering
	if (this->settings->isScreenDisabled == false) {
	  this->displayChangeCount += this->pourDisplay->Tick();
	  this->displayChangeCount += this->totpDisplay->Tick();
	}

  return 0;
}

void KegeratorStateMachine::NfcLoop() {
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

void KegeratorStateMachine::Initialize(DeviceSettings *settings) {
  this->settings = settings;

  // Setup taps
  if (this->taps != NULL) {
    delete[] this->taps;
  }

  uint8_t tapCount = this->settings->tapCount;
  this->taps = new Tap[tapCount];
  for (int i = 0; i < tapCount; i++) {
    this->taps[i].Setup(
      this,
      settings->tapIds[i],
      settings->pulsesPerGallon[i],
			settings->timeForValveOpen
    );
  }

  this->sensors->Setup(this, this->taps, tapCount);
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

	// Apply settings
	RGB.brightness(this->settings->ledBrightness);
	this->display->DimScreen(this->settings->isScreenDisabled);
	this->openValveTimer.SetDuration(this->settings->secondsToStayOpen * 1000 + 10000);

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

void KegeratorStateMachine::SetStateFromDeviceStatus() {
  if (this->settings->deviceStatus == DeviceStatus::INACTIVE) {
    this->SetState(KegeratorState::INACTIVE);
  }
  else if (this->settings->deviceStatus == DeviceStatus::CLEANING) {
    this->SetState(KegeratorState::CLEANING);
  }
	else if (this->settings->deviceStatus == DeviceStatus::UNLOCKED) {
    this->SetState(KegeratorState::UNLOCKED);
  }
	else if (this->settings->deviceStatus == DeviceStatus::CONFIGURE) {
    this->SetState(KegeratorState::CONFIGURE);
  }
  else {
    this->SetState(KegeratorState::LISTENING);
  }
}

int KegeratorStateMachine::Settings(String data) {
  this->SetState(KegeratorState::INITIALIZING);

  return 0;
}

int KegeratorStateMachine::StartPour(String data) {
	// Split the pour data here
	// shouldOpenSolenoid
	// maxOunces
	// shouldStayOpenUntilMaxOunces
  this->lastAuthorizedToken = data;

  // This is only really necessary when anonymous pours are turned off
  // When anon pours are enabled, the solenoid isn't used.
  this->sensors->OpenSolenoids();
  this->pourResponseStartTime = millis();
  this->SetState(KegeratorState::POUR_AUTHORIZED);

  return 0;
}

void KegeratorStateMachine::TapStartedPouring(ITap &tap) {
  this->SetState(KegeratorState::POURING);
  Serial.println("Started Pouring");

  if (this->lastAuthorizedToken != NULL && this->lastAuthorizedToken.length()) {
    tap.SetAuthToken(this->lastAuthorizedToken);
  }

  this->lastAuthorizedToken = "";
	this->CleanupTapState();
}

void KegeratorStateMachine::TapStoppedPouring(
  uint32_t tapID,
  uint32_t totalPulses,
  String authenticationKey,
	uint32_t pourStartTime,
	uint32_t pourEndTime
) {
  this->CleanupTapState();
  Serial.println("Finished Pouring");

  if (totalPulses > PULSE_EPSILON) {
    this->serverLink->SendPourToServer(
      tapID,
      totalPulses,
      authenticationKey,
			this->totpDisplay->GetTOTP(),
			pourStartTime,
			pourEndTime
    );

    // TODO - Maybe show pulse on display a bit longer...?
  }

}

void KegeratorStateMachine::StopPouring() {
  for (int i = 0; i < this->settings->tapCount; i++) {
    if (this->taps[i].IsPouring()) {
      this->taps[i].StopPour();
    }
  }
}

void KegeratorStateMachine::Timeout() {
  this->lastAuthorizedToken = "";
  this->CleanupTapState();
}

void KegeratorStateMachine::CleanupTapState() {
  bool allStopped = true;
  for (uint8_t ii = 0; ii < this->settings->tapCount; ii++) {
    if (this->taps[ii].IsPouring()) {
      allStopped = false;
    }
    else {
      this->sensors->CloseSolenoid(ii);
      this->sensors->ResetFlowSensor(ii);
    }
  }

  if (allStopped) {
    this->SetStateFromDeviceStatus();
  }
}
