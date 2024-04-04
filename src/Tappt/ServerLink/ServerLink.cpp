#include "ServerLink.h"

String getNextToken(String input, int *start, String delimiter) {
  int end = input.indexOf(delimiter, *start);
  String data = input.substring(*start, end);
  *start = end + delimiter.length();

  return data;
}

ServerLink::ServerLink(IKegeratorStateMachine *kegeratorStateMachine) {
  this->kegeratorStateMachine = kegeratorStateMachine;
  this->settings.tapIds = NULL;
  String deviceID = System.deviceID();

  Particle.subscribe("hook-response/tappt_initialize-" + deviceID,
                     &ServerLink::Initialize, this, MY_DEVICES);

  // Called by server when device settings are updated.
  Particle.function("settings", &ServerLink::Settings, this);

  // Called by server when user tries to pour.
  Particle.function("pour", &ServerLink::Pour, this);
  // Response when token is used
  Particle.subscribe("hook-response/tappt_request-pour-" + deviceID,
                     &ServerLink::PourResponse, this, MY_DEVICES);

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
  Serial.print("Error: ");
  Serial.println(message);
}

void ServerLink::Initialize(const char *event, const char *data) {
  Serial.println("Initializing");

  if (strlen(data) <= 0) {
    SetError("Empty Data");
    return;
  }

  Serial.println(data);

  this->initializeTimer.stop();

  String response = String(data);
  String delimiter = "~";
  int start = 1;
  int end = response.indexOf(delimiter, start);

  if (end == -1) {
    SetError("Bad Input");
    return;
  }

  this->settings.deviceId = getNextToken(response, &start, delimiter);
  this->settings.authorizationToken = getNextToken(response, &start, delimiter);

  String tapIds = getNextToken(response, &start, delimiter);

  this->settings.deviceStatus =
      getNextToken(response, &start, delimiter).toInt();

  String pulsesPerGallon = getNextToken(response, &start, delimiter);

  this->settings.ledBrightness =
      getNextToken(response, &start, delimiter).toInt();
  this->settings.isTOTPDisabled =
      getNextToken(response, &start, delimiter) == "true";
  this->settings.isScreenDisabled =
      getNextToken(response, &start, delimiter) == "true";
  this->settings.timeForValveOpen =
      getNextToken(response, &start, delimiter).toInt();
  this->settings.secondsToStayOpen =
      getNextToken(response, &start, delimiter).toInt();
  this->settings.shouldInvertScreen =
      getNextToken(response, &start, delimiter) == "true";
  this->settings.nfcStatus = getNextToken(response, &start, delimiter).toInt();

  // Build out Tap IDs
  delimiter = ",";
  start = 0;
  int tapCount = 0;

  String tapResult;
  while (true) {
    tapResult = getNextToken(tapIds, &start, delimiter);
    if (tapResult == "") {
      break;
    }

    tapCount++;
  }

  this->settings.tapCount = tapCount;

  if (this->settings.tapIds != NULL) {
    delete[] this->settings.tapIds;
  }
  this->settings.tapIds = new uint32_t[tapCount];
  start = 0;
  int iter = 0;
  while (tapCount > 0 && iter < tapCount) {
    this->settings.tapIds[iter] =
        getNextToken(tapIds, &start, delimiter).toInt();
    iter++;
  }

  // End Tap IDs
  start = 0;
  iter = 0;

  if (this->settings.pulsesPerGallon != NULL) {
    delete[] this->settings.pulsesPerGallon;
  }

  this->settings.pulsesPerGallon = new uint32_t[tapCount];
  while (tapCount > 0 && iter < tapCount) {
    this->settings.pulsesPerGallon[iter] =
        getNextToken(pulsesPerGallon, &start, delimiter).toInt();
    iter++;
  }

  // End pulsesPerGallon IDs
  this->kegeratorStateMachine->Initialize(&this->settings);

  Serial.println("Initialized");
  Serial.print("LED Brightness ");
  Serial.println(this->settings.ledBrightness, DEC);
  Serial.print("TOTP Disabled ");
  Serial.println(this->settings.isTOTPDisabled, DEC);
  Serial.print("Screen Disabled ");
  Serial.println(this->settings.isScreenDisabled, DEC);
  Serial.print("Time For Valve Open ");
  Serial.println(this->settings.timeForValveOpen, DEC);
  Serial.print("Seconds To Stay Open ");
  Serial.println(this->settings.secondsToStayOpen, DEC);
  Serial.print("Should Invert Screen ");
  Serial.println(this->settings.shouldInvertScreen, DEC);
  Serial.print("NFC Status ");
  Serial.println(this->settings.nfcStatus, DEC);
  Serial.print("Tap Count ");
  Serial.println(this->settings.tapCount, DEC);
}

int ServerLink::Settings(String data) {
  this->CallInitialize();
  Serial.println("Settings");

  return 0;
}

void ServerLink::AuthorizePour(uint32_t deviceId, String authenticationKey) {
  Serial.println(authenticationKey);
  Serial.println("printed");

  sprintf(json, "{\"authToken\":\"%s\",\"id\":\"%lu\",\"tkn\":\"%s\"}",
          this->settings.authorizationToken.c_str(), deviceId,
          // remove \u0002 and "en"
          authenticationKey.substring(3).c_str());

  Serial.print("Request Pour");
  Serial.println(json);
  Particle.publish("tappt_request-pour", json, 5, PRIVATE);
}

int ServerLink::Pour(String data) {
  Serial.print("Pour: ");
  Serial.println(data);

  if (data.length() <= 0) {
    return -1;
  }

  int start = 0;
  String delimiter = "~";
  String token = getNextToken(data, &start, delimiter);

  // TODO: Remove this once the firmware has rolled out for a while
  if (token == data) {
    this->kegeratorStateMachine->StartPour(token, 0, NULL);
    return 0;
  }

  int constraintCount = getNextToken(data, &start, delimiter).toInt();
  if (constraintCount == 0) {
    this->kegeratorStateMachine->StartPour(token, 0, NULL);
    return 0;
  }

  int iter = 0;
  TapConstraint *constraints = new TapConstraint[constraintCount];
  while (iter < constraintCount) {
    String constraintString = getNextToken(data, &start, delimiter);

    int constraintStart = 0;
    TapConstraint &constraint = constraints[iter];
    constraint.tapIndex =
        getNextToken(constraintString, &constraintStart, ",").toInt();
    constraint.type =
        getNextToken(constraintString, &constraintStart, ",").toInt();
    constraint.pulses =
        getNextToken(constraintString, &constraintStart, ",").toInt();

    iter++;
  }

  this->kegeratorStateMachine->StartPour(token, constraintCount, constraints);
  delete[] constraints;
  return 0;
}

void ServerLink::PourResponse(const char *event, const char *data) {
  this->Pour(String(data));
}

void ServerLink::SendPourToServer(uint32_t tapId, uint32_t totalPulses,
                                  String authenticationKey, String totp,
                                  uint32_t pourMilliseconds) {
  sprintf(json,
          "{\"authToken\":\"%s\",\"tapId\":\"%u\",\"pourKey\":\"%s\",\"totp\":"
          "\"%s\",\"pulses\":\"%u\",\"start\":\"%u\",\"end\":\"%u\"}",
          this->settings.authorizationToken.c_str(), tapId,
          authenticationKey != NULL && authenticationKey.length()
              ? authenticationKey.c_str()
              : "",
          totp.c_str(), totalPulses,
          Time.now() - (uint32_t)(pourMilliseconds / 1000), Time.now());

  Serial.print("Finished Pour");
  Serial.println(json);
  Particle.publish("tappt_pour-finished", json, 60, PRIVATE);
}
