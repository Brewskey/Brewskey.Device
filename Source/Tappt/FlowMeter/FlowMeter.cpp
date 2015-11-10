#include "FlowMeter.h"

void FlowMeter::FlowCounter()
{
	static uint8_t buffer = 0;
	uint8_t pin = digitalRead(FLOW_PIN);

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
}

FlowMeter::FlowMeter(Solenoid *solenoid, LED* led)
{
  this->solenoid = solenoid;
	this->led = led;
	pinMode(FLOW_PIN, INPUT);
	digitalWrite(FLOW_PIN, HIGH);
  //attachInterrupt(FLOW_PIN, &FlowMeter::FlowCounter, this, FALLING);
	Particle.function("pour", &FlowMeter::StartPour, this);

	//this->StopPour();
}

int FlowMeter::StartPour(String data)
{
  this->solenoid->Open();

	this->timer.Reset();
  flowCount = 0;
  this->lastFlowCount = 0;
  this->pouring = true;
  this->waitCount = 0;
	this->pourKey = String(data);
	Serial.print("Start Pour");Serial.println(pourKey);
	return 0;
}

void FlowMeter::StopPour()
{
	this->pourKey = "";
	this->solenoid->Close();
	this->pouring = false;
	this->led->SetColor(0,0,0);
}

int FlowMeter::Tick()
{
  if (this->pouring == false) {
    return -1;
  }

	this->led->SetColor(0,255,0);
	this->FlowCounter();

	this->timer.Tick();

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
		sprintf(
	    json,
	    "{\"pourKey\":\"%s\",\"pulses\":\"%d\"}",
	    this->pourKey.c_str(),
			this->flowCount
	  );

		this->StopPour();

		Serial.print("Finished Pour");Serial.println(json);
		Particle.publish("tappt_pour-finished", json, 60, PRIVATE);

		return 0;
  }

	return 1;
}
