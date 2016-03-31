#include "Display.h"

Display::Display(): display(OLED_RESET) {
  this->hasChanges = false;
  this->shouldAnimate = false;
  this->shouldRenderQR = false;

  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  display.clearDisplay();
  display.drawBitmap(3, 12, IMG_ICON, 32, 40, 1);
  display.drawBitmap(40, 21, IMG_LOGO_TEXT, 88, 32, 1);
  display.display();
  delay(1);
}

void Display::UpdateQR() {
  this->shouldRenderQR = true;
  this->hasChanges = true;
}

void Display::UpdateTOTP(String totp) {
  this->shouldRenderTOTP = true;
  this->hasChanges = true;

  this->totp = totp;
}

int Display::Tick() {
  if (!this->hasChanges && !this->shouldAnimate) {
    return 0;
  }

  this->hasChanges = false;
  display.clearDisplay();

  display.drawBitmap(3, 12, IMG_ICON, 32, 40, 1);

  if (this->shouldRenderQR) {
    this->RenderQR();
  }

  if (this->shouldRenderTOTP) {
    this->RenderTOTP();
  }

  display.display();
  delay(1);
}

void Display::RenderQR() {
  /* data */
	int x, y;
  for (y = 0; y < WD; y++) {
  	for (x = 0; x < WD; x++) {
      if (!QRBIT(x,y)) {
        continue;
      }

      int scale = 1;
      int xPos = x * scale;
      int yPos = y * scale;

      for (int xx = 0; xx < scale; xx++) {
        for (int yy = 0; yy < scale; yy++) {
          display.drawPixel(xPos + xx, yPos + yy, WHITE);
        }
      }
  	}
  }
}

void Display::RenderTOTP() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(42,26);

  for (int i = 0; i < totp.length(); i++) {
    display.write(totp[i]);
  }
  this->shouldRenderTOTP = false;
}
