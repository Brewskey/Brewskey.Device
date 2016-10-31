#ifndef TotpDisplay_h
#define TotpDisplay_h

#include "ITick.h"
#include "Tap.h"
#include "Display.h"
#include "TOTP.h"
#include "DeviceSettings.h"

class TotpDisplay: public ITick  {
public:
  TotpDisplay(Display* display);

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
