#ifndef Sensors_h
#define Sensors_h

#include "math.h"

#include "ISolenoids.h"
#include "Tappt/Pins.h"
#include "Tappt/ITick.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/Temperature/Temperature.h"
#include "Tappt/KegeratorStateMachine/IKegeratorStateMachine.h"
#include "Tappt/KegeratorStateMachine/KegeratorState.h"

#ifdef EXPANSION_BOX_PIN
#include "Tappt/Packets/ConfigurationPacket.h"
#include "Tappt/Packets/StandardSendPacket.h"
#include "Tappt/Packets/PacketReader.h"

#endif

class Sensors : public ISolenoids {
public:
  #ifdef EXPANSION_BOX_PIN
  Sensors(PacketReader &packetReader);
  #else
  Sensors();
  #endif
  void Setup(IKegeratorStateMachine* stateMachine, Tap* taps, uint8_t tapCount);
  virtual int Tick();
  virtual void OpenSolenoid(uint8_t solenoid);
  virtual void OpenSolenoids();
  virtual void CloseSolenoid(uint8_t solenoid);
  virtual void CloseSolenoids();
  virtual void ResetFlowSensor(uint8_t solenoid);
  virtual void SetState(KegeratorState::e state);

#ifdef EXPANSION_BOX_PIN
  void ReadMultitap();
#endif
private:
  void SingleFlowCounter();
  void ParsePourPacket();
  void ParseConfigurationPacket();

  Temperature* temperatureSensor = NULL;
  Tap* taps = NULL;

  uint8_t tapCount;
  IKegeratorStateMachine* stateMachine;
  KegeratorState::e state = KegeratorState::INITIALIZING;

#ifdef EXPANSION_BOX_PIN
  ConfigurationPacket configPacket;
  StandardSendPacket* sendPackets = NULL;
  PacketReader &reader;
#endif
  uint8_t boxCount = 0;
  uint8_t sendIndex = 0;

  // start true so we wait to send first packet
  bool isWaitingForResponse = true;
  TapptTimer packetResponseTimer = TapptTimer(400);
};

#endif
