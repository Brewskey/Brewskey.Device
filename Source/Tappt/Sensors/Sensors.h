#ifndef Sensors_h
#define Sensors_h

#include "ISolenoids.h"
#include "Tappt/Pins.h"
#include "Tappt/ITick.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/Temperature/Temperature.h"

#ifdef EXPANSION_BOX_PIN
#include "Tappt/Packets/StandardSendPacket.h"
#include "Tappt/Packets/PacketReader.h"

#endif

class Sensors : public ISolenoids, public ITick {
public:
  Sensors(PacketReader &packetReader);
  void Setup(Tap* taps, uint8_t tapCount);
  virtual int Tick();
  virtual void OpenSolenoid(uint8_t solenoid);
  virtual void OpenSolenoids();
  virtual void CloseSolenoid(uint8_t solenoid);
  virtual void CloseSolenoids();
  virtual void ResetFlowSensor(uint8_t solenoid);
#ifdef EXPANSION_BOX_PIN
  void ReadMultitap();
#endif
private:
  void SingleFlowCounter();

  Temperature* temperatureSensor = NULL;
  Tap* taps = NULL;

  uint8_t tapCount;

#ifdef EXPANSION_BOX_PIN
  StandardSendPacket sendPacket = StandardSendPacket(0x01);
  PacketReader &reader;

  // start true so we wait to send first packet
  bool isWaitingForResponse = true;
  TapptTimer packetResponseTimer = TapptTimer(400);
#endif
};

#endif
