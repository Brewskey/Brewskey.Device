#include "Sensors.h"

#ifdef EXPANSION_BOX_PIN
Sensors::Sensors(PacketReader &packetReader) : reader(packetReader)
#else
Sensors::Sensors()
#endif
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
  /*set RS485 direction pin LOW: receiver*/
  digitalWrite(EXPANSION_BOX_PIN, LOW);
#endif
}

void Sensors::Setup(
  IKegeratorStateMachine* stateMachine,
  Tap* taps,
  uint8_t tapCount
) {
  if (this->temperatureSensor == NULL) {
    this->temperatureSensor = new Temperature();
  }
  this->stateMachine = stateMachine;
  this->taps = taps;
  this->tapCount = tapCount;

  this->boxCount = ceil((float)tapCount / MAX_TAP_COUNT_PER_BOX);

  #ifdef EXPANSION_BOX_PIN
  if (this->sendPackets != NULL) {
    delete[] this->sendPackets;
  }
  this->sendPackets = new StandardSendPacket[this->boxCount]();
  for (int i = 0; i < this->boxCount; i++) {
    this->sendPackets[i].SetDestination(i + 1);
  }

  this->isWaitingForResponse = false;
  while (Serial1.available()) {
    Serial1.read();
  }
  this->packetResponseTimer.Start();
  #endif
}

int Sensors::Tick() {
  this->temperatureSensor->Tick();
  // TODO: Maybe we can support 5 taps...?
#if USE_INTERRUPT == 0
  this->SingleFlowCounter();
#endif
#ifdef EXPANSION_BOX_PIN
  if (this->sendPackets == NULL) {
    return 0;
  }

  this->packetResponseTimer.Tick();
  if (this->packetResponseTimer.ShouldTrigger()) {
    this->isWaitingForResponse = false;
    this->reader.Reset();
  }

  if (this->state == KegeratorState::CONFIGURE) {
    if (this->isWaitingForResponse) {
      return 0;
    }

    if (!this->configPacket.IsReady()) {
      return 0;
    }

    this->packetResponseTimer.Stop();

    Serial.println("Send config");
    this->configPacket.Send();

    this->stateMachine->OnConfigureNextBox(this->configPacket.GetDestination());
    this->isWaitingForResponse = true;

    return 0;
  }

  if (
    !this->isWaitingForResponse
  ) {
    Serial.println("Send");
    this->sendPackets[this->sendIndex].Send();
    this->sendIndex++;
    this->sendIndex = this->sendIndex % this->boxCount;
    this->isWaitingForResponse = true;
  }
#endif
  return 0;
}

void Sensors::SetState(KegeratorState::e state)
{
  Serial.print("Sensor::SetState ");
  Serial.println(state);
  #ifdef EXPANSION_BOX_PIN
  if (this->state != state)
  {
    // Begin configuration
    if (state == KegeratorState::CONFIGURE) {
      Serial.println("SetupConfigurationPackets");
      this->sendIndex = 0;
      this->configPacket.SetDestination(0);
    }

    // Use default send packet
    if (this->state == KegeratorState::CONFIGURE) {
      this->packetResponseTimer.Start();
      Serial.println("SetupDefaultSendPackets");
    }
  }
  #endif

  this->state = state;
}

void Sensors::SingleFlowCounter()
{
  if (this->state == KegeratorState::CONFIGURE)
  {
    return;
  }
  uint8_t pin = digitalRead(FLOW_PIN);
#if USE_INTERRUPT == 1
  // delayMicroseconds(1200);
  if (pin == 0) {
    this->taps[0].SetTotalPulses(this->taps[0].GetTotalPulses() + 1);
  }
#else
  static uint8_t buffer = 0;

  /*shift buffer byte by 1 */
  buffer <<= 1;

  /*SENSOR_PIN represents the pin status, if high set last bit in buffer to 1
  else it will remain 0*/
  if (pin != 0)
  {
    buffer |= 0x01;
  }

  /*check for 0x07 pattern (mask upper 2 bits), representing a low to high
  transition verified by 3 low samples followed by 3 high samples*/
  if ((buffer & 0x3F) == 0x07)
  {
    this->taps[0].SetTotalPulses(this->taps[0].GetTotalPulses() + 1);
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
  if (this->state == KegeratorState::CONFIGURE) {
    return;
  }

  if (solenoid == 0) {
    digitalWrite(SOLENOID_PIN, LOW);
  }

#ifdef EXPANSION_BOX_PIN
  if (this->sendPackets == NULL) {
    return;
  }

  uint8_t packetIndex = floor((float)solenoid / MAX_TAP_COUNT_PER_BOX);
  this->sendPackets[packetIndex].CloseSolenoid(solenoid % MAX_TAP_COUNT_PER_BOX);
#endif
}

void Sensors::OpenSolenoids() {
  for (int i = 0; i < this->boxCount * MAX_TAP_COUNT_PER_BOX; i++) {
    this->OpenSolenoid(i);
  }
}

void Sensors::OpenSolenoid(uint8_t solenoid) {
  if (this->state == KegeratorState::CONFIGURE) {
    return;
  }

  digitalWrite(SOLENOID_PIN, HIGH);
#ifdef EXPANSION_BOX_PIN
  if (this->sendPackets == NULL) {
    return;
  }
  uint8_t packetIndex = floor((float)solenoid / MAX_TAP_COUNT_PER_BOX);
  this->sendPackets[packetIndex].OpenSolenoid(solenoid % MAX_TAP_COUNT_PER_BOX);
#endif
}

void Sensors::ResetFlowSensor(uint8_t tap) {
  if (this->state == KegeratorState::CONFIGURE) {
    return;
  }

#ifdef EXPANSION_BOX_PIN
  if (this->sendPackets == NULL) {
    return;
  }
  uint8_t packetIndex = floor((float)tap / MAX_TAP_COUNT_PER_BOX);
  this->sendPackets[packetIndex].ResetFlowSensor(tap % MAX_TAP_COUNT_PER_BOX);
#endif
}

#ifdef EXPANSION_BOX_PIN
void Sensors::ReadMultitap(void)
{
  if (!this->isWaitingForResponse) {
    return;
  }
  this->reader.Read();

  if (!this->reader.IsPacketReady()) {
    return;
  }

  bool isValid = this->reader.IsValid();
  this->reader.Reset();
  if (!isValid) {
    this->isWaitingForResponse = false;
    return;
  }

  uint8_t destination = this->reader.GetDestination();
  uint8_t packetType = this->reader.GetPacketType();

  if (destination != 0x00) {
    this->isWaitingForResponse = false;
    return;
  }

  if (packetType == CONFIGURATION_RESPONSE_PACKET_TYPE)
  {
    this->ParseConfigurationPacket();
  }
  else if (packetType == POUR_PACKET_TYPE)
  {
    this->ParsePourPacket();
  }

  this->isWaitingForResponse = false;
}

void Sensors::ParseConfigurationPacket()
{
  uint8_t* incomingBuffer = this->reader.GetDataBuffer();
  uint32_t pulses =
    (incomingBuffer[0] << 24) |
    (incomingBuffer[1] << 16) |
    (incomingBuffer[2] << 8) |
    (incomingBuffer[3]);

  Serial.print("Parsed Config Response: ");
  Serial.println(pulses);

  this->configPacket.PrepareNextResponse(pulses);
}

void Sensors::ParsePourPacket()
{
  uint8_t* incomingBuffer = this->reader.GetDataBuffer();
  uint8_t ii;

  uint8_t source = this->reader.GetSource();
  const uint8_t FLOW_START = 1;

  for (ii = 0; ii < MAX_TAP_COUNT_PER_BOX; ii++) {
    uint32_t pulses =
      (incomingBuffer[FLOW_START + 4 * ii] << 24) |
      (incomingBuffer[FLOW_START + 4 * ii + 1] << 16) |
      (incomingBuffer[FLOW_START + 4 * ii + 2] << 8) |
      (incomingBuffer[FLOW_START + 4 * ii + 3]);

    uint8_t tapIndex = (source - 1) * MAX_TAP_COUNT_PER_BOX + ii;
    if (tapIndex >= this->tapCount) {
      if (pulses > 0) {
        Serial.print("Pour occurred on tap that isn't set up: ");
        Serial.print(tapIndex + 1); // Plus one since people don't expect zero-based
        Serial.println();
        this->ResetFlowSensor(ii);
      }
      continue;
    }

    // Get difference to determine if it is still pouring
    uint32_t totalPulses = this->taps[tapIndex].GetTotalPulses();
    if (pulses == totalPulses) {
      continue;
    }

    this->taps[tapIndex].SetTotalPulses(pulses);
  }

#if SHOW_OUTPUT
  Serial.printf("SOL1: %s, PULSES1: %lu, SOL2: %s, PULSES2: %lu, SOL3: %s, PULSES3: %lu, SOL4: %s, PULSES4: %lu\n",
    (incomingBuffer[0] & 0x03) ? "ON" : "OFF",	/*solenoid 1*/
    (incomingBuffer[1] << 24) | (incomingBuffer[2] << 16) | (incomingBuffer[3] << 8) | (incomingBuffer[4]), /*flow 1*/
    (incomingBuffer[0] & 0x0C) ? "ON" : "OFF",	/*solenoid 2*/
    (incomingBuffer[5] << 24) | (incomingBuffer[6] << 16) | (incomingBuffer[7] << 8) | (incomingBuffer[8]), /*flow 2*/
    (incomingBuffer[0] & 0x30) ? "ON" : "OFF",	/*solenoid 3*/
    (incomingBuffer[9] << 24) | (incomingBuffer[10] << 16) | (incomingBuffer[11] << 8) | (incomingBuffer[12]), /*flow 3*/
    (incomingBuffer[0] & 0xC0) ? "ON" : "OFF",	/*solenoid 4*/
    (incomingBuffer[13] << 24) | (incomingBuffer[14] << 16) | (incomingBuffer[15] << 8) | (incomingBuffer[16])); /*flow 4*/
  Serial.println();
#endif
}

#endif
