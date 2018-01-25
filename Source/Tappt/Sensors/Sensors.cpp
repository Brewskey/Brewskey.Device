#include "Sensors.h"

#define UART_SEND_LENGTH 8
#define UART_SOLENOID_ON 4
#define UART_SOLENOID_OFF 5
#define UART_RESET_FLOW 6
#define INCOMING_BUFFER_SIZE 21

Sensors::Sensors()
{
  pinMode(FLOW_PIN, INPUT);
  digitalWrite(FLOW_PIN, HIGH);
#if USE_INTERRUPT == 1
  attachInterrupt(FLOW_PIN, &Sensors::SingleFlowCounter, this, FALLING, 0);
#endif
  pinMode(SOLENOID_PIN, OUTPUT);

#ifdef EXPANSION_BOX_PIN
  /*RS485 direction pin*/
  pinMode(EXPANSION_BOX_PIN, OUTPUT);

#endif
}

void Sensors::Setup(Tap taps[], uint8_t tapCount) {
  if (this->temperatureSensor == NULL) {
    this->temperatureSensor = new Temperature();
  }
  this->taps = taps;
  this->tapCount = tapCount;
}

int Sensors::Tick() {
  this->temperatureSensor->Tick();
  // TODO: Maybe we can support 5 taps...?
#if USE_INTERRUPT == 0
  this->SingleFlowCounter();
#endif
#ifdef EXPANSION_BOX_PIN
  this->ReadMultitap();
#endif
  return 0;
}

void Sensors::SingleFlowCounter()
{
	uint8_t pin = digitalRead(FLOW_PIN);
#if USE_INTERRUPT == 1
	// delayMicroseconds(1200);
	if (pin == 0) {
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

// If this gets called it means the pour stopped or the user changed the state
// of the device (cleaning mode/disabled/enabled)
void Sensors::CloseSolenoids() {
  for (int ii = 0; ii < this->tapCount; ii++) {
    this->CloseSolenoid(ii);
  }
}

void Sensors::CloseSolenoid(uint8_t solenoid) {
  if (solenoid == 0) {
    digitalWrite(SOLENOID_PIN, LOW);
  }

#ifdef EXPANSION_BOX_PIN
  this->sendPacket.CloseSolenoid(solenoid);
#endif
}

void Sensors::OpenSolenoids() {
  for (int i = 0; i < 4; i++) {
    this->OpenSolenoid(i);
  }
}

void Sensors::OpenSolenoid(uint8_t solenoid) {
  digitalWrite(SOLENOID_PIN, HIGH);
#ifdef EXPANSION_BOX_PIN
  this->sendPacket.OpenSolenoid(solenoid);
#endif
}

void Sensors::ResetFlowSensor(uint8_t solenoid) {
#ifdef EXPANSION_BOX_PIN
  this->sendPacket.ResetFlowSensor(solenoid);
#endif
}

#ifdef EXPANSION_BOX_PIN
void Sensors::ReadMultitap(void)
{
	uint8_t ii,
    count = 0,
    checksum = 0,
    data = 0,
    esc_flag = 0,
    isValid =0;

  this->sendPacket.Send();
  delay(100);

	/*read all received bytes*/
	while (Serial1.available() > 0) {
		data = Serial1.read();			/* Get data */
		if(data == '#' && !esc_flag)				/* If finding first escape byte */
		{
			esc_flag = 1;							/* Set escape byte flag */
		}
		else
		{
      /* Escape byte not set */
			if(!esc_flag)
			{
        /* Getting sync byte of packet, since no escape byte beore it */
				if(data == '+')
				{
					count = 0;						/* Reset Counter - since start of packet */
					for(ii = 0; ii < PACKET_BUFFER; ii++)
					{
						incomingBuffer[ii] = 0;	/* Clearing packet buffer */
					}

					continue;
				}

				if(data == '-')						/* End of packet */
				{
					checksum = 0;					/* Reset checksum */

					for(ii = 0; ii < INCOMING_BUFFER_SIZE; ii++)		/* Calculating checksum of packet */
					{
						checksum ^= incomingBuffer[ii];
					}

					checksum = 255 - checksum;

					if(checksum == incomingBuffer[count - 1])
					{
						isValid = 1;	/*packet is valid*/
					}
					else
					{
						isValid = 0;			/* packet invalid */
					}
				}
			}
			else
			{
				esc_flag = 0;
			}

      /* If count still less than packet buffer size */
			if(count < PACKET_BUFFER)
			{
				incomingBuffer[count] = data;	/* Store data in buffer */
				count++;									/* Increment counter */
			}
		}
	}

	if(isValid)
	{
    /*print received data to USB*/
		if(incomingBuffer[0] == 0x00 && incomingBuffer[1] == 0x01 && incomingBuffer[2] == 0x33)
		{
      const uint8_t FLOW_START = 5;
      uint8_t tapsInBox = this->tapCount % 4;
      for (ii = 0; ii < MAX_TAP_COUNT_PER_BOX; ii++) {
        uint32_t pulses =
          (incomingBuffer[FLOW_START + 4 * ii]<<24) |
          (incomingBuffer[FLOW_START + 4 * ii + 1]<<16) |
          (incomingBuffer[FLOW_START + 4 * ii + 2]<<8) |
          (incomingBuffer[FLOW_START + 4 * ii + 3]);

        if (ii > tapsInBox) {
          if (pulses > 0) {
            Serial.print("Pour occurred on tap that isn't set up: ");
            Serial.print(ii);
            Serial.println();
          }
          continue;
        }

        // Get difference to determine if it is still pouring
        uint32_t totalPulses = this->taps[ii].GetTotalPulses();
        if (pulses == totalPulses) {
          continue;
        }
        long difference = pulses - totalPulses;
        if (difference <= 0) {
          continue;
        }

        this->taps[ii].AddToFlowCount(difference);
      }

#if SHOW_OUTPUT
			Serial.printf("SOL1: %s, PULSES1: %lu, SOL2: %s, PULSES2: %lu, SOL3: %s, PULSES3: %lu, SOL4: %s, PULSES4: %lu\n",
				(incomingBuffer[4] & 0x01)?"ON":"OFF",	/*solenoid 1*/
				(incomingBuffer[5]<<24) | (incomingBuffer[6]<<16) |	(incomingBuffer[7]<<8) | (incomingBuffer[8]), /*flow 1*/
				(incomingBuffer[4] & 0x02)?"ON":"OFF",	/*solenoid 2*/
				(incomingBuffer[9]<<24) | (incomingBuffer[10]<<16) |	(incomingBuffer[11]<<8) | (incomingBuffer[12]), /*flow 2*/
				(incomingBuffer[4] & 0x04)?"ON":"OFF",	/*solenoid 3*/
				(incomingBuffer[13]<<24) | (incomingBuffer[14]<<16) |	(incomingBuffer[15]<<8) | (incomingBuffer[16]), /*flow 3*/
				(incomingBuffer[4] & 0x08)?"ON":"OFF",	/*solenoid 4*/
				(incomingBuffer[17]<<24) | (incomingBuffer[18]<<16) |	(incomingBuffer[19]<<8) | (incomingBuffer[20])); /*flow 4*/
      Serial.println();
#endif
		}
	}
}

#endif
