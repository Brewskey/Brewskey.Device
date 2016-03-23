#ifndef Display_h
#define Display_h

#define OLED_RESET 4

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ITick.h"
#include "qrencode.h"

#include "logo.h"
#include "logo_text.h"

class Display: public ITick {
public:
  Display();
  virtual int Tick();

  void UpdateQR();
  void UpdateTOTP(String totp);
private:
  void RenderQR();
  void RenderTOTP();

  Adafruit_SSD1306 display;

  bool hasChanges;
  bool shouldAnimate;
  bool shouldRenderQR;
  bool shouldRenderTOTP;

  String totp;
};

#endif
