#ifndef TotpDisplay_h
#define TotpDisplay_h

#include "Tappt/ITick.h"
#include "Tappt/Tap/Tap.h"
#include "Tappt/Display/Display.h"
#include "TOTP/TOTP.h"
#include "Tappt/ServerLink/DeviceSettings.h"

class TotpDisplay : public ITick {
public:
  TotpDisplay(Display* display);

  void Reset();
  void Setup(DeviceSettings* settings, Tap* taps, uint8_t tapCount);
  String GetTOTP() { return this->currentTotp; }
  virtual int Tick();
private:
  Display * display;
  Tap* taps;
  DeviceSettings* settings;
  uint8_t tapCount;

  int lastPourCount;
  String currentTotp;
  bool logoCurrentlyDrawn;
};

#endif
