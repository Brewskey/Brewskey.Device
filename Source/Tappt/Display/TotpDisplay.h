#ifndef TotpDisplay_h
#define TotpDisplay_h

#include "Tappt/ITick.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/Display/Display.h"
#include "TOTP/TOTP.h"
#include "Tappt/ServerLink/DeviceSettings.h"

class TotpDisplay: public ITick  {
public:
  TotpDisplay(Display* display);

  void Reset();
  void Setup(DeviceSettings* settings, Tap* taps, int tapCount);
  virtual int Tick();
private:
  Display* display;
  Tap* taps;
  DeviceSettings* settings;
  int tapCount;

  int lastPourCount;
  String currentTotp;
  bool logoCurrentlyDrawn;
};

#endif
