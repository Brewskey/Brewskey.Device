#include "ConfigurationPacket.h"

#define DATA_PACKET_SIZE 4

ConfigurationPacket::ConfigurationPacket()
    : PacketBase(DATA_PACKET_SIZE, CONFIGURATION_PACKET_TYPE) {}

bool ConfigurationPacket::IsReady() { return this->isReady; }

void ConfigurationPacket::PrepareNextResponse(uint32_t pulses) {
  if (pulses == 0) {
    return;
  }

  this->isReady = true;

  this->destination++;
  this->SetDestination(this->destination);
  this->dataPacket[3] = (uint8_t)(pulses >> 24);
  this->dataPacket[4] = (uint8_t)(pulses >> 16);
  this->dataPacket[5] = (uint8_t)(pulses >> 8);
  this->dataPacket[6] = (uint8_t)(pulses);
}

void ConfigurationPacket::ResetDataPacket() {
  this->isReady = false;

  // Set the pulses back to zero
  this->dataPacket[3] = 0x00;
  this->dataPacket[4] = 0x00;
  this->dataPacket[5] = 0x00;
  this->dataPacket[6] = 0x00;
}
