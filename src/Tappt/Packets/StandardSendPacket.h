#pragma once

#include "Tappt/Packets/PacketBase.h"
#include "Tappt/Pins.h"

class StandardSendPacket : public PacketBase {
 public:
  StandardSendPacket();

  void CloseSolenoids();
  void OpenSolenoids();
  void CloseSolenoid(uint8_t solenoid);
  void OpenSolenoid(uint8_t solenoid);
  void ResetFlowSensor(uint8_t solenoid);

  virtual void ResetDataPacket();
};
