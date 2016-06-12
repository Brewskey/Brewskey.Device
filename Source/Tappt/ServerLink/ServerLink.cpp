#include "ServerLink.h"

ServerLink::ServerLink(IStateManager *stateManager) {
  //this->stateManager = stateManager;

  String deviceID = System.deviceID();

	// Called by server when device settings are updated.
	Particle.function("settings", &ServerLink::Settings, this);

	// Called by server when user tries to pour.
	Particle.function("pour", &ServerLink::Pour, this);


  this->initializeTimer.start();
  this->CallInitialize();
}

void ServerLink::CallInitialize() {
  Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);
}

void ServerLink::InitializeComplete(const char* event, const char* data) {
  if (strlen(data) <= 0) {
    return;
  }

  this->initializeTimer.stop();

  char strBuffer[90] = "";
  String(data).toCharArray(strBuffer, 90);

  this->settings.deviceId = String(strtok(strBuffer, "~"));
  this->settings.authorizationToken = String(strtok((char*)NULL, "~"));
  this->settings.tapIds = String(strtok((char*)NULL, "~"));
  this->settings.deviceStatus = String(strtok((char*)NULL, "~"));

  Serial.print("Tap IDs: ");
  Serial.println(this->settings.tapIds);

  Serial.print("Device Status: ");
  Serial.println(this->settings.deviceStatus);

  this->stateManager->Initialize(&this->settings);
}

int ServerLink::Settings(String data) {
	Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);
	return 0;
}

int ServerLink::Pour(String data) {
	Serial.print("Pour: ");
	Serial.println(data);

	if (data.length() <= 0) {
		return -1;
	}

  this->stateManager->Pour(data);

	return 0;
}

void ServerLink::PourResponse(const char* event, const char* data) {
	this->Pour(String(data));
}
