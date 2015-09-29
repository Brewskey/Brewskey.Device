#include "FlowMeter.h"

volatile int flowCount = 0;
void flowCounter()
{
	flowCount++;
}

FlowMeter::FlowMeter(Solenoid *solenoid)
{
  this->solenoid = solenoid;
  attachInterrupt(FLOW_PIN, flowCounter, FALLING);
}

void FlowMeter::StartPour()
{
  this->solenoid->Open();
  flowCount = 0;
  this->lastFlowCount = 0;
  this->pouring = true;
  this->waitCount = 0;
}

void FlowMeter::StopPour()
{
	this->solenoid->Close();
	this->pouring = false;

	// send pour to server!!!
  Serial.print("flow: ");Serial.println(flowCount);
}

int FlowMeter::Tick()
{
  if (!this->pouring) {
    return -1;
  }

  if ((millis() - this->pourTimer) < 1000) {
    return 0;
  }

  this->pourTimer = millis();

  int count = flowCount - this->lastFlowCount;
  this->lastFlowCount = flowCount;

  if (count <= PULSE_EPSILON) {
    this->waitCount++;
  } else {
		this->waitCount = 0;
		return 0;
	}

  int maxWait = flowCount > PULSE_EPSILON ? 3 : 5;
  if (this->waitCount > maxWait)
  {
    this->StopPour();
  }

	return 0;
}
