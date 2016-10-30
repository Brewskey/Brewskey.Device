#ifndef PourDisplay_h
#define PourDisplay_h

#include "ITick.h"
#include "ITap.h"
#include "Display.h"

class PourDisplay: public ITick  {
public:
  PourDisplay(Display* display);

  void Setup(ITap* taps, int tapCount);
  virtual int Tick();
private:
  void SetEmptySlotForTap(int tapId);
  int GetXOffset(String text, int offsetType, int fontSize);


  Display* display;
  ITap* taps;
  int tapCount;

  int currentPouringTaps[4] = {-1, -1, -1, -1};
  String currentDisplays[4];
};

#endif
