#ifndef Sensors_h
#define Sensors_h

#include "ISolenoids.h"
#include "Tappt/Pins.h"
#include "Tappt/ITick.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/Temperature/Temperature.h"

/*maximum size of incoming packet from ext board*/
#define PACKET_BUFFER	64

class Sensors: public ISolenoids, public ITick {
public:
  Sensors(Tap* taps, uint8_t tapCount);
  virtual int Tick();
  virtual void OpenSolenoid(uint8_t solenoid);
  virtual void OpenSolenoids();
  virtual void CloseSolenoid(uint8_t solenoid);
  virtual void CloseSolenoids();
  virtual void ResetFlowSensor(uint8_t solenoid);
private:
  void SingleFlowCounter();
#ifdef EXPANSION_BOX_PIN
  void ReadMultitap();
  void UartSendPacket(uint8_t* pstr, int length);
  void PrepareDataPacket();
  void ResetDataPacket();
#endif
  Temperature* temperatureSensor;
  Tap* taps;

  uint8_t tapCount;

#ifdef EXPANSION_BOX_PIN
  /*array for outging packet*/
  uint8_t dataPacket[16];

  /*array for incoming packet*/
  uint8_t incomingBuffer[PACKET_BUFFER];
#endif
};

#endif
