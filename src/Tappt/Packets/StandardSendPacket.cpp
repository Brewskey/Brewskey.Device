#include "StandardSendPacket.h"

uint8_t TAP_BITS[4] = {0x03, 0x0C, 0x30, 0xC0};

#define DATA_PACKET_SIZE 3

#ifdef USE_BETA_PACKET_FORMAT
#define SOLENOID_ON_INDEX 4
#define SOLENOID_OFF_INDEX 5
#define RESET_FLOW_INDEX 6
#else
#define SOLENOID_ON_INDEX 3
#define SOLENOID_OFF_INDEX 4
#define RESET_FLOW_INDEX 5
#endif

StandardSendPacket::StandardSendPacket()
    : PacketBase(DATA_PACKET_SIZE, FLOW_CONTROL_PACKET_TYPE) {
  // Set version packet for beta hardware
  // Beta hardware had an extra byte :/
#ifdef USE_BETA_PACKET_FORMAT
  this->dataPacket[3] = 0x01; /*mainboard packet version */
#endif
}

// If this gets called it means the pour stopped or the user changed the state
// of the device (cleaning mode/disabled/enabled)
void StandardSendPacket::CloseSolenoids() {
  for (uint8_t ii = 0; ii < MAX_TAP_COUNT_PER_BOX; ii++) {
    this->CloseSolenoid(ii);
  }
}

void StandardSendPacket::OpenSolenoids() {
  for (uint8_t ii = 0; ii < MAX_TAP_COUNT_PER_BOX; ii++) {
    this->OpenSolenoid(ii);
  }
}

void StandardSendPacket::CloseSolenoid(uint8_t solenoid) {
  this->dataPacket[SOLENOID_OFF_INDEX] |= TAP_BITS[solenoid];
}

void StandardSendPacket::OpenSolenoid(uint8_t solenoid) {
  this->dataPacket[SOLENOID_ON_INDEX] |= TAP_BITS[solenoid];
}

void StandardSendPacket::ResetFlowSensor(uint8_t sensor) {
  this->dataPacket[RESET_FLOW_INDEX] |= TAP_BITS[sensor];
}

void StandardSendPacket::ResetDataPacket() {
  /* turn solenoid ON
  Bits: 0x03 - solendoid 1
  Bits: 0x0C - solendoid 2
  Bits: 0x30 - solendoid 3
  Bits: 0xC0 - solendoid 4

  solendoid will turn OFF automatically when no more flow is detected*/
  this->dataPacket[SOLENOID_ON_INDEX] = 0x00;

  /* force solenoid OFF
  Bits: 0x03 - solendoid 1
  Bits: 0x0C - solendoid 2
  Bits: 0x30 - solendoid 3
  Bits: 0xC0 - solendoid 4

  This can be used to override the auto-off algorithm, for example in case
  abnormal flow is detected (tap left open or leak)
  */
  this->dataPacket[SOLENOID_OFF_INDEX] = 0x00;

  /* reset flow sensors
  Bits: 0x03 - Flow Sensor 1
  Bits: 0x0C - Flow Sensor 2
  Bits: 0x30 - Flow Sensor 3
  Bits: 0xC0 - Flow Sensor 4

  This is used to reset the flow sensor.
  */
  this->dataPacket[RESET_FLOW_INDEX] = 0x00;
}
