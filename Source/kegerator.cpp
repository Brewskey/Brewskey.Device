#include "application.h"

#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"

#define SOLENOID_PIN (D2)

PN532_SPI pn532spi(SCK, MISO, MOSI, SS);
NfcAdapter nfc = NfcAdapter(pn532spi);

#define FLOW_PIN (D0)
#define PULSE_EPSILON 0

void flowCounter();

byte state, waitCount;
volatile int flowCount = 0;
unsigned long pourTimer, totalPulses;

void setup(void) {
    Serial.begin(115200);


    while(!Serial.available()) {
      Spark.process();
    }


    pinMode(SOLENOID_PIN, OUTPUT);

    Serial.println("NDEF Reader");
    //nfc.begin();

    attachInterrupt(FLOW_PIN, flowCounter, FALLING);
}

void loop(void) {
    /*Serial.println("\r\nScan a NFC tag\r\n");
    if (nfc.tagPresent())
    {
        digitalWrite(SOLENOID_PIN, HIGH);
        NfcTag tag = nfc.read();
        tag.print();
    }

    delay(5000);
    digitalWrite(SOLENOID_PIN, LOW); */

    Serial.print("P:");
    Serial.println(flowCount);
}

/*
void pour() {
  if ((millis() - time) >= 1000) {
		detachInterrupt(FLOW);
		time = millis();

		if (flowCount == PULSE_EPSILON) {
			waitCount++;
			int maxWait = totalPulses > PULSE_EPSILON ? 3 : 5;
			if (waitCount > maxWait)
			{
				state = DONE_POURING;
			}

		} else {
			waitCount = 0;
			Serial.print("P:");
			Serial.println(flowCount);
			totalPulses += flowCount;
			flowCount = 0;
		}
		attachInterrupt(FLOW, flowCounter, FALLING);
	}
}
*/

void flowCounter() {
Serial.println("++");
	flowCount++;
}
