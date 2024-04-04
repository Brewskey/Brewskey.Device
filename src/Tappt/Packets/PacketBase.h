#pragma once

#include "Tappt/Packets/PacketDefinitions.h"
#include "Tappt/Pins.h"
#include "application.h"

class PacketBase {
 public:
  PacketBase(uint8_t packetSize, uint8_t packetType);
  uint8_t GetPacketSize() { return this->packetSize; }
  void Send();
  void SetDestination(uint8_t destination);
  uint8_t GetDestination();

  virtual ~PacketBase();

 protected:
  virtual void PrepareDataPacket();

  // Packets only need to be sent once to devices so this will reset all the
  // data.
  virtual void ResetDataPacket() = 0;

  uint8_t *dataPacket = NULL;

 private:
  uint8_t packetSize = 0;
};
