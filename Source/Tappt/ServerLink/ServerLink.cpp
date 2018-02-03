#include "ServerLink.h"

ServerLink::ServerLink(IStateManager *stateManager) {
  this->stateManager = stateManager;
  this->settings.tapIds = NULL;
  String deviceID = System.deviceID();

  Particle.subscribe(
    "hook-response/tappt_initialize-" + deviceID,
    &ServerLink::Initialize,
    this,
    MY_DEVICES
  );

  // Called by server when device settings are updated.
  Particle.function("settings", &ServerLink::Settings, this);

  // Called by server when user tries to pour.
  Particle.function("pour", &ServerLink::Pour, this);
  // Response when token is used
  Particle.subscribe(
    "hook-response/tappt_request-pour-" + deviceID,
    &ServerLink::PourResponse,
    this,
    MY_DEVICES
  );

  this->CallInitialize();
}

void ServerLink::CallInitialize() {
  this->initializeTimer.start();
  Serial.println("Getting device settings");
  Particle.publish("tappt_initialize", (const char *)0, 10, PRIVATE);
}

void SetError(String message) {
  RGB.control(true);
  RGB.color(56, 56, 128);
  Serial.println(message);
}

void ServerLink::Initialize(const char* event, const char* data) {
  Serial.println("Initializing");

  if (strlen(data) <= 0) {
    SetError("Empty Data");
    return;
  }

  this->initializeTimer.stop();

  String response = String(data);
  String delimeter = "~";
  int start = 1;
  int end = response.indexOf(delimeter, start);

  if (end == -1) {
    SetError("Bad Input");
    return;
  }

  this->settings.deviceId = response.substring(start, end);
  start = end + delimeter.length();
  end = response.indexOf(delimeter, start);

  this->settings.authorizationToken = response.substring(start, end);
  start = end + delimeter.length();
  end = response.indexOf(delimeter, start);

  String tapIds = response.substring(start, end);
  start = end + delimeter.length();
  end = response.indexOf(delimeter, start);

  this->settings.deviceStatus = response.substring(start, end).toInt();
  start = end + delimeter.length();
  end = response.indexOf(delimeter, start);

  String pulsesPerGallon = response.substring(start, end);

  // Build out Tap IDs
  delimeter = ",";
  start = 0;
  end = tapIds.indexOf(delimeter);
  int tapCount = 0;

  while (end > 0) {
    start = end + delimeter.length();
    end = tapIds.indexOf(delimeter, start);
    tapCount++;
  }

  this->settings.tapCount = tapCount;

  if (this->settings.tapIds != NULL) {
    delete[] this->settings.tapIds;
  }
  this->settings.tapIds = new uint32_t[tapCount];
  start = 0;
  end = tapIds.indexOf(delimeter);
  int iter = 0;
  while (end >= 0 && tapCount > 0 && iter < tapCount) {
    this->settings.tapIds[iter] = tapIds.substring(start, end).toInt();
    start = end + delimeter.length();
    end = tapIds.indexOf(delimeter, start);
    iter++;
  }

  // End Tap IDs
  start = 0;
  end = pulsesPerGallon.indexOf(delimeter);
  iter = 0;

  if (this->settings.pulsesPerGallon != NULL) {
    delete[] this->settings.pulsesPerGallon;
  }

  this->settings.pulsesPerGallon = new uint32_t[tapCount];
  while (end >= 0 && tapCount > 0 && iter < tapCount) {
    this->settings.pulsesPerGallon[iter] =
      pulsesPerGallon.substring(start, end).toInt();
    start = end + delimeter.length();
    end = pulsesPerGallon.indexOf(delimeter, start);
    iter++;
  }

  // End pulsesPerGallon IDs

  this->stateManager->Initialize(&this->settings);
  Serial.println("Initialized");
}

int ServerLink::Settings(String data) {
  this->CallInitialize();
  Serial.println("Settings");

  return 0;
}

void ServerLink::AuthorizePour(uint32_t deviceId, String authenticationKey) {
  Serial.println(authenticationKey);
  Serial.println("printed");

  sprintf(
    json,
    "{\"authToken\":\"%s\",\"id\":\"%lu\",\"tkn\":\"%s\"}",
    this->settings.authorizationToken.c_str(),
    deviceId,
    // remove \u0002 and "en"
    authenticationKey.substring(3).c_str()
  );

  Serial.print("Request Pour"); Serial.println(json);
  Particle.publish("tappt_request-pour", json, 5, PRIVATE);
}

int ServerLink::Pour(String data) {
  Serial.print("Pour: ");
  Serial.println(data);

  if (data.length() <= 0) {
    return -1;
  }

  this->stateManager->StartPour(data);

  return 0;
}

void ServerLink::PourResponse(const char* event, const char* data) {
  this->Pour(String(data));
}

void ServerLink::SendPourToServer(
  uint32_t tapId,
  uint32_t totalPulses,
  String authenticationKey
) {
  sprintf(
    json,
    "{\"authToken\":\"%s\",\"tapId\":\"%lu\",\"pourKey\":\"%s\",\"pulses\":\"%d\"}",
    this->settings.authorizationToken.c_str(),
    tapId,
    authenticationKey != NULL && authenticationKey.length()
    ? authenticationKey.c_str()
    : "",
    totalPulses
  );


  Serial.print("Finished Pour"); Serial.println(json);
  Particle.publish("tappt_pour-finished", json, 60, PRIVATE);
}
