#include "Display.h"

Display::Display():
#ifdef OLED_SPI
display(OLED_DC, OLED_RESET, OLED_CS)
#else
display(OLED_RESET)
#endif
{
  uint8_t iter;
#ifdef OLED_SPI
  display.begin(SSD1306_SWITCHCAPVCC);
#else
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
#endif
  display.clearDisplay();
  display.drawBitmap(0, 0, IMG_LOGO_TEXT, 128, 64, 1);

  display.display();
  delay(1);
}

void Display::BeginBatch() {
  display.clearDisplay();
}

void Display::DrawIcon(int color) {
  display.drawBitmap(3, 12, IMG_ICON, 32, 40, color);
}

void Display::SetText(
  String text,
  uint8_t x,
  uint8_t y,
  uint8_t size,
  int offsetType,
  int color
) {
  int offset = this->GetXOffset(text, offsetType, size);
  display.setTextSize(size);
  display.setTextColor(color);
  display.setCursor((int)x + offset, y);

  for (int i = 0; i < text.length(); i++) {
    display.write(text[i]);
  }
}

void Display::ClearText(
  String text,
  uint8_t x,
  uint8_t y,
  uint8_t size,
  int offsetType
) {
  this->SetText(text, x, y, size, offsetType, BLACK);
}

void Display::EndBatch() {
  display.display();
  delay(1);
}

int Display::GetXOffset(String text, int offsetType, int fontSize) {
  if (offsetType == 0) {
    return 0;
  }

  int offset = text.length() * 6 * fontSize;
  if (offsetType == 2) {
    return -(int)round(offset / 2.0);
  }

  return -offset;
}
