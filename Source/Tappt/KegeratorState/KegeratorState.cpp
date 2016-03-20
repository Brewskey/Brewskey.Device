#include "KegeratorState.h"
#include "qrencode.h"

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)

KegeratorState::KegeratorState(NfcClient* nfcClient, FlowMeter* flowMeter) {
  this->nfcClient = nfcClient;
  this->flowMeter = flowMeter;

	String deviceID = System.deviceID();

  Particle.subscribe(
		"hook-response/tappt_initialize-" + deviceID,
		&KegeratorState::Initialized,
		this,
		MY_DEVICES
	);

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
	WITH_LOCK(Serial) {
		Serial.print("Auth Token: ");
		Serial.println(this->authorizationToken);

		TOTP totp = TOTP(
			(uint8_t*)this->authorizationToken.c_str(),
			this->authorizationToken.length()
		);
		const char* newCode = totp.getCode((long)Time.now());

		Serial.print("TOTP: ");
		Serial.println(newCode);


		String wifiDetails = "WIFI:S:<ssid>;T:<wep|WPDA>;P;<password>;;";
		QRcode *qr = QRcode_encodeString(
			wifiDetails,
			0,
			QR_ECLEVEL_M,
			QR_MODE_8,
			true
		);

		Serial.println("WIDTH: ");
		Serial.println(qr->width);
		Serial.println();

		int i_qr, j_qr;
		for (i_qr = 0; i_qr < qr->width; i_qr++) {
		    for (j_qr = 0; j_qr < qr->width; j_qr++) {
		        if (qr->data[(i_qr * qr->width) + j_qr] & 0x1)
		            Serial.print("*");
		        else
		            Serial.print(" ");
		    }
		    Serial.println();
		}
		Serial.println();
		Serial.println();

/*
		unsigned char encoded[80];
		memset(encoded, 0, sizeof(encoded));
		int width = EncodeData(QR_LEVEL_M, QR_VERSION_S, newCode, 20, encoded);

		Serial.print("Data length: ");
		Serial.println(width);
		Serial.print("Data: ");
		for (int i = 0; i < 80; i++) {
			Serial.print(encoded[i]);
		}
		Serial.println();
		*/
  }
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
  }

  return 0;
}

void KegeratorState::Initialized(const char* event, const char* data) {
  if (strlen(data) <= 0) {
    return;
  }

	String str = String(data);
  char strBuffer[64] = "";
  str.toCharArray(strBuffer, 64);

  this->deviceId = String(strtok(strBuffer, "~"));
	this->authorizationToken = String(strtok((char*)NULL, "~"));
  this->nfcClient->Initialize(this->deviceId);

	this->ledTimer.start();
}

int KegeratorState::Pour(String data) {
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
