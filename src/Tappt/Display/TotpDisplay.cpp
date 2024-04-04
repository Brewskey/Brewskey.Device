#include "TotpDisplay.h"

int TOTP_SIZE = 2;

TotpDisplay::TotpDisplay(Display* display) {
  this->display = display;
  this->settings = NULL;
  this->tapCount = 0;
}

void TotpDisplay::Setup(DeviceSettings* settings, Tap* taps, uint8_t tapCount) {
  this->taps = taps;
  this->tapCount = tapCount;
  this->settings = settings;
}

void TotpDisplay::Reset() { this->currentTotp = ""; }

bool drawn = false;
int TotpDisplay::Tick() {
  int currentPourCount = 0;
  for (int i = 0; i < this->tapCount; i++) {
    currentPourCount += this->taps[i].IsPouring() ? 1 : 0;
  }

  TOTP totp = TOTP((uint8_t*)this->settings->authorizationToken.c_str(),
                   this->settings->authorizationToken.length());
  String newTotp = String(totp.getCode((long)Time.now()));

  if (this->lastPourCount == currentPourCount && this->currentTotp == newTotp) {
    return 0;
  }

  // Clear old suff;
  int changeCount = 0;
  bool iconCleared;
  bool logoCleared;

  // TODO - refactor this logic. It's way overcomplicated and has bugs in it
  if (currentPourCount != 0 && this->lastPourCount == 0 &&
      this->settings->isTOTPDisabled) {
    this->display->DrawLogo(BLACK);
    this->display->DrawIcon(WHITE);
    changeCount++;
    logoCleared = true;
  }
  if (this->lastPourCount <= 1 && currentPourCount > 1) {
    this->display->DrawIcon(BLACK);
    iconCleared = true;
    changeCount++;
  }

  if (((currentPourCount == 0 && this->lastPourCount != 0) ||
       this->currentTotp.length() == 0) &&
      this->settings->isTOTPDisabled) {
    this->display->DrawIcon(BLACK);
    this->display->DrawLogo(WHITE);
    changeCount++;
  } else if (this->currentTotp.length() == 0 ||
             currentPourCount <= 1 && this->lastPourCount > 1 &&
                 !this->settings->isTOTPDisabled) {
    this->display->DrawIcon(WHITE);
    changeCount++;
  }

  if (this->settings->isTOTPDisabled == false) {
    bool totpCleared = false;
    if (iconCleared || this->currentTotp != newTotp) {
      int x = 42;
      int y = 24;
      int offsetType = 0;

      if (this->lastPourCount == 1) {
        x = 42;
        y = 10;
      } else if (this->lastPourCount > 1) {
        x = 64;
        y = 24;
        offsetType = 2;
      }

      this->display->ClearText(this->currentTotp, x, y, TOTP_SIZE, offsetType);

      totpCleared = true;
      changeCount++;
    }

    if (this->currentTotp != newTotp || totpCleared) {
      int x = 42;
      int y = 24;
      int offsetType = 0;

      if (currentPourCount == 1) {
        x = 42;
        y = 10;
      } else if (currentPourCount > 1) {
        x = 64;
        y = 24;
        offsetType = 2;
      }
      // just render TOTP + LOGO
      this->display->SetText(newTotp, x, y, TOTP_SIZE, offsetType);

      changeCount++;
    }
  }

  this->currentTotp = newTotp;
  this->lastPourCount = currentPourCount;

  return changeCount;
}
