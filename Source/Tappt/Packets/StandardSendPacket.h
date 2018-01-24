#pragma once

#include "Tappt/Pins.h"
#include "Tappt/Packets/PacketBase.h"

class StandardSendPacket: public PacketBase {
public:
  StandardSendPacket(uint8_t destination, bool hasVersionBit = false);

  void CloseSolenoids();
  void OpenSolenoids();
  void CloseSolenoid(uint8_t solenoid);
  void OpenSolenoid(uint8_t solenoid);
  void ResetFlowSensor(uint8_t solenoid);

  virtual void ResetDataPacket();

private:
  uint8_t offset = 0;
};
