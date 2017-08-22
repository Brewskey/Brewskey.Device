#ifndef Display_h
#define Display_h

#include "Pins.h"
#ifdef BIG_SCREEN
#include "Adafruit_SH1106.h"
#else
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#endif
#include "ITick.h"

#include "logo.h"
#include "logo_text.h"

class Display {
public:
  Display();

  void BeginBatch();
  void DrawIcon(int color);
  void EndBatch();
  // void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  //   this->display.fillRect(x,y,w,h,color);
  // };

  void ClearText(
    String text,
    uint8_t x,
    uint8_t y,
    uint8_t size = 2,
    int offsetType = 0
  );
  void SetText(
    String text,
    uint8_t x,
    uint8_t y,
    uint8_t size = 2,
    int offsetType = 0,
    int color = WHITE
  );

private:
  int GetXOffset(String text, int offsetType, int fontSize);
  void WriteString(String text);

#ifdef BIG_SCREEN
  Adafruit_SH1106 display;
#else
  Adafruit_SSD1306 display;
#endif
};

#endif
