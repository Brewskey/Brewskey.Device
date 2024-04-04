#include "Display.h"

#ifdef BIG_SCREEN
#define SWITCHCAPVCC SH1106_SWITCHCAPVCC
#define I2C_ADDRESS SH1106_I2C_ADDRESS
#else
#define SWITCHCAPVCC SSD1306_SWITCHCAPVCC
#define I2C_ADDRESS SSD1306_I2C_ADDRESS
#endif

Display::Display()
    :
#ifdef OLED_SPI
#ifdef SPI_HW_MODE
      display(OLED_DC, OLED_RESET, OLED_CS)
#else
      display(MOSI, SCK, OLED_DC, OLED_RESET, OLED_CS)
#endif
#else
      display(OLED_RESET)
#endif
{
#ifdef OLED_SPI
  display.begin(SWITCHCAPVCC);
#else
  display.begin(SWITCHCAPVCC, I2C_ADDRESS);
#endif
  display.clearDisplay();
  this->DrawLogo(WHITE);
  display.display();
  delay(1);
}

void Display::DimScreen(bool shouldDim) { display.dim(shouldDim); }
void Display::InvertScreen(bool shouldInvert) {
  display.invertDisplay(shouldInvert);
}

void Display::BeginBatch() { display.clearDisplay(); }

void Display::DrawIcon(int color) {
  display.drawBitmap(3, 12, IMG_ICON, 32, 40, color);
}

void Display::DrawLogo(int color) {
  display.drawBitmap(0, 0, IMG_LOGO_TEXT, 128, 64, color);
}

void Display::SetText(String text, uint8_t x, uint8_t y, uint8_t size,
                      int offsetType, int color) {
  int offset = this->GetXOffset(text, offsetType, size);
  display.setTextSize(size);
  display.setTextColor(color);
  display.setCursor((int)x + offset, y);

  for (int i = 0; i < text.length(); i++) {
    display.write(text[i]);
  }
}

void Display::ClearText(String text, uint8_t x, uint8_t y, uint8_t size,
                        int offsetType) {
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
