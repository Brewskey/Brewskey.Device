#ifndef PourDisplay_h
#define PourDisplay_h

#include "Tappt/Display/Display.h"
#include "Tappt/ITick.h"
#include "Tappt/Tap/Tap.h"

class PourDisplay : public ITick {
 public:
  PourDisplay(Display* display);

  void Setup(Tap* taps, uint8_t tapCount);
  virtual int Tick();

 private:
  void SetEmptySlotForTap(int tapId);

  Display* display;
  Tap* taps;
  uint8_t tapCount;

  int currentPouringTaps[4] = {-1, -1, -1, -1};
  String currentDisplays[4];
};

#endif
