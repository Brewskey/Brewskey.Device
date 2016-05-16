#include "FlowMeter.h"
#include "Pins.h"

void FlowMeter::FlowCounter()
{
	uint8_t pin = digitalRead(FLOW_PIN);
#ifdef USE_INTERRUPT
	delayMicroseconds(1200);
	if (pin == 0) {
		flowCount++;
	}
#else
	static uint8_t buffer = 0;

	/*shift buffer byte by 1 */
	buffer <<= 1;

	/*SENSOR_PIN represents the pin status, if high set last bit in buffer to 1 else it will remain 0*/
	if(pin != 0)
	{
		buffer |= 0x01;
	}

	/*check for 0x07 pattern (mask upper 2 bits), representing a low to high transition verified by 3 low samples followed by 3 high samples*/
	if((buffer & 0x3F) == 0x07)
	{
		flowCount++;
	}
#endif
}

FlowMeter::FlowMeter(Solenoid *solenoid, Display *display)
{
  this->solenoid = solenoid;
	this->display = display;



	this->StopPour();
}

int FlowMeter::StartPour(String data)
{
	this->solenoid->Open();

	this->timer.Reset();
  this->flowCount = 0;
	this->lastOunceString = "";
  this->lastFlowCount = 0;
  this->pouring = true;
  this->waitCount = 0;
	this->pourKey = data;
	Serial.print("Start Pour");Serial.println(pourKey);

	pinMode(FLOW_PIN, INPUT);
	digitalWrite(FLOW_PIN, HIGH);
	attachInterrupt(FLOW_PIN, &FlowMeter::FlowCounter, this, FALLING, 0);

#if USE_FAKE_POUR == 1
this->flowCount = 400;
#endif

	this->display->BeginBatch();
	this->display->SetText("Start", 53, 15);
	this->display->SetText("Pouring", 42, 35);
	this->display->EndBatch();

	return 0;
}

void FlowMeter::StopPour()
{
	detachInterrupt(FLOW_PIN);
	this->pourKey = "";
	this->solenoid->Close();
	this->pouring = false;
}

int FlowMeter::Tick()
{
  if (this->pouring == false) {
    return -1;
  }

	#ifndef USE_INTERRUPT
	this->FlowCounter();
	#endif

	this->timer.Tick();

	char ounceString[7];
	float ounces = (float)this->flowCount * (float)128 / (float)10313;

	sprintf(
    ounceString,
    "%.1f oz",
    ounces
  );

	if (String(ounceString) != this->lastOunceString) {
		Serial.println("Drawing");
		if (this->lastOunceString.length() == 0) {
			this->display->BeginBatch();
		}

		this->lastOunceString = String(ounceString);
		this->display->FillRect(42, 26, 12 * 7, 16, 0);
		this->display->SetText(ounceString, 42, 26);
		this->display->EndBatch();
	}

  if (!this->timer.ShouldTrigger) {
    return 1;
  }

	Serial.println("Pouring");

  int count = this->flowCount - this->lastFlowCount;
  this->lastFlowCount = this->flowCount;

  if (count <= PULSE_EPSILON) {
    this->waitCount++;
  } else {
		this->waitCount = 0;
		return 1;
	}

  int maxWait = this->flowCount > PULSE_EPSILON ? 3 : 5;
  if (this->waitCount > maxWait)
  {
		// Don't log it... not enough pulses.
		if (this->flowCount <= PULSE_EPSILON) {
			Serial.println("Not enough pulses. Ending pour " + this->pourKey);
			this->StopPour();

			this->display->BeginBatch();
			this->display->SetText("Pour", 53, 15);
			this->display->SetText("Failed", 42, 35);
			this->display->EndBatch();

			return 0;
		}

		sprintf(
	    json,
	    "{\"pourKey\":\"%s\",\"pulses\":\"%d\"}",
	    this->pourKey.c_str(),
			this->flowCount
	  );

		this->display->BeginBatch();
		this->display->SetText("Done", 58, 15);
		this->display->SetText("Pouring", 42, 35);
		this->display->EndBatch();

		Serial.print("Finished Pour");Serial.println(json);
		Particle.publish("tappt_pour-finished", json, 60, PRIVATE);

		this->StopPour();

		return 0;
  }

	return 1;
}
