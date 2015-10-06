#include "FlowMeter.h"

void FlowMeter::FlowCounter()
{
	flowCount++;
}

FlowMeter::FlowMeter(Solenoid *solenoid)
{
  this->solenoid = solenoid;
  attachInterrupt(FLOW_PIN, &FlowMeter::FlowCounter, this, FALLING);
	Particle.function("pour", &FlowMeter::StartPour, this);
}

int FlowMeter::StartPour(String pourKey)
{
  this->solenoid->Open();
	this->timer.Reset();
  flowCount = 0;
  this->lastFlowCount = 0;
  this->pouring = true;
  this->waitCount = 0;
	this->pourKey = pourKey;
	Serial.print("Start Pour");Serial.println(pourKey);
}

void FlowMeter::StopPour()
{
	this->pourKey = "";
	this->solenoid->Close();
	this->pouring = false;
}

int FlowMeter::Tick()
{
  if (!this->pouring) {
    return -1;
  }

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
