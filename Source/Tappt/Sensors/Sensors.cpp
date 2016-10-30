#include "Sensors.h"


Sensors::Sensors(ITap taps[], uint8_t tapCount) {
  this->taps = taps;
  this->tapCount = tapCount;

  this->temperatureSensor = new Temperature();

  pinMode(FLOW_PIN, INPUT);
	digitalWrite(FLOW_PIN, HIGH);
#if USE_INTERRUPT == 1
	attachInterrupt(FLOW_PIN, &Sensors::SingleFlowCounter, this, FALLING, 0);
#endif
  pinMode(SOLENOID_PIN, OUTPUT);

  this->OpenSolenoids();
  delay(10);
  this->CloseSolenoids();
}

int iter = 0;
int Sensors::Tick() {
  this->temperatureSensor->Tick();

  if (this->tapCount == 1) {
    this->SingleFlowCounter();
    return 0;
  }

  Serial.println(this->tapCount);
  Serial.println((unsigned int)&this->taps, HEX);
  Serial.println((unsigned int)&this->taps[1], HEX);


  iter++;
  if (iter == 200) {
    iter = 0;
    for (int ii = 0; ii < 2; ii++) {
      uint8_t pulses = 0; // TODO - Read from external board
      this->taps[ii].IsPouring();
      //this->taps[ii].AddToFlowCount(1 + ii);
    }
  }
  return 0;
}

void Sensors::SingleFlowCounter()
{
	uint8_t pin = digitalRead(FLOW_PIN);
#if USE_INTERRUPT == 1
	delayMicroseconds(1200);
	if (pin == 0) {
    iter = 0;
    this->taps[0].AddToFlowCount(1);
	}
#else
	static uint8_t buffer = 0;

	/*shift buffer byte by 1 */
	buffer <<= 1;

	/*SENSOR_PIN represents the pin status, if high set last bit in buffer to 1
  else it will remain 0*/
	if(pin != 0)
	{
		buffer |= 0x01;
	}

	/*check for 0x07 pattern (mask upper 2 bits), representing a low to high
  transition verified by 3 low samples followed by 3 high samples*/
	if((buffer & 0x3F) == 0x07)
	{
		this->taps[0].AddToFlowCount(1);
	}
#endif
}

void Sensors::CloseSolenoids() {
  for (int i = 0; i < this->tapCount; i++) {
    this->CloseSolenoid(i);
  }
}

void Sensors::CloseSolenoid(uint8_t solenoid) {
  if (solenoid == 0) {
    digitalWrite(SOLENOID_PIN, LOW);
  }

  // TODO - Handle multiple solenoids
}

void Sensors::OpenSolenoids() {
  digitalWrite(SOLENOID_PIN, HIGH);

  // TODO - Handle multiple solenoids
}
