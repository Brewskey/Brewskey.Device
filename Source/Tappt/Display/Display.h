#ifndef Display_h
#define Display_h

#include "Pins.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "ITick.h"

#include "logo.h"
#include "logo_text.h"

class Display {
public:
  Display();

  void BeginBatch(bool showLogo = true);
  void EndBatch();
  void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    this->display.fillRect(x,y,w,h,color);
  };

  void ClearText(String text, uint8_t x, uint8_t y, uint8_t size = 2);
  void SetText(
    String text,
    uint8_t x,
    uint8_t y,
    uint8_t size = 2,
    int color = WHITE
  );

private:
  void WriteString(String text);

  Adafruit_SSD1306 display;
};

#endif
