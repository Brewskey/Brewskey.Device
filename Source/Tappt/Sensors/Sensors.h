#ifndef Sensors_h
#define Sensors_h

#include "ISolenoids.h"
#include "ITick.h"
#include "ITap.h"
#include "Temperature.h"

class Sensors: public ISolenoids {
public:
  Sensors(ITap* taps, uint8_t tapCount);
  virtual void OpenForTap(ITap &tap);
  virtual void CloseForTap(ITap &tap);
private:
  void SingleFlowCounter();
  int GetTapIndex(ITap &tap);

  Temperature* temperatureSensor;
  ITap* taps;

  uint8_t tapCount;
};

#endif
