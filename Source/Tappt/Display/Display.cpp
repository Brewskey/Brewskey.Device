#include "Display.h"

Display::Display():
#ifdef OLED_SPI
display(OLED_DC, OLED_RESET, OLED_CS)
#else
display(OLED_RESET)
#endif
{
  uint8_t iter;
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  display.clearDisplay();
  display.drawBitmap(0, 0, IMG_LOGO_TEXT, 128, 64, 1);

  display.display();
  delay(1);
}

void Display::BeginBatch(bool showLogo) {
  display.clearDisplay();

  if (showLogo) {
    display.drawBitmap(3, 12, IMG_ICON, 32, 40, 1);
  }
}

void Display::SetText(
  String text,
  uint8_t x,
  uint8_t y,
  uint8_t size,
  int color
) {
  display.setTextSize(size);
  display.setTextColor(color);
  display.setCursor(x, y);

  for (int i = 0; i < text.length(); i++) {
    display.write(text[i]);
  }
}

void Display::ClearText(
  String text,
  uint8_t x,
  uint8_t y,
  uint8_t size
) {
  this->SetText(text, x, y, size, BLACK);
}

void Display::EndBatch() {
  display.display();
  delay(1);
}
