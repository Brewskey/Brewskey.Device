#pragma once

#include "application.h"
#include "Tappt/Pins.h"

#define DESTINATION_INDEX 0
#define PACKET_SOURCE_INDEX 1
#define PACKET_TYPE_INDEX 2

class PacketBase {
public:
  PacketBase(uint8_t packetSize, uint8_t packetType);
  uint8_t GetPacketSize() { return this->packetSize; }
  void Send();

  virtual ~PacketBase();
protected:
  void SetDestination(uint8_t destination);
  virtual void PrepareDataPacket();

  // Packets only need to be sent once to devices so this will reset all the
  // data.
  virtual void ResetDataPacket() = 0;

  uint8_t *dataPacket = NULL;
private:
  uint8_t packetSize = 0;
};
