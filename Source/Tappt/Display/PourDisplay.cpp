#include "PourDisplay.h"

/*
* 1. Slot
*
* 1. x
* 2. y
* 3. font-size
* 4. Offset type
*    0 = none
*    1 = text width
*    2 = half text width
*/
int TEXT_SETTINGS_1[1][4] = {
  {0, 0, 2, 0},
};


int TEXT_SETTINGS_4[4][4] = {
  {0, 0, 1, 0},
  {128, 0, 1, 1},
  {0, 56, 1, 0},
  {128, 56, 1, 1},
};

PourDisplay::PourDisplay(Display* display) {
  this->display = display;
}

void PourDisplay::Setup(Tap* taps, int tapCount) {
  this->taps = taps;
  this->tapCount = tapCount;
}

int PourDisplay::Tick() {
  // Clean up display state.
  int changeCount = 0;
  int counter = 0;
  for(int i = 0; i < this->tapCount; i++) {
    if (this->taps[i].IsPouring()) {
      counter++;
      this->SetEmptySlotForTap(i);
    }
	}

  // No reson to do any logic if no tap is pouring
  if (counter == 0) {
    return 0;
  }
/*
  int (*textSettings)[4] = TEXT_SETTINGS_4;
  if (counter == 1) {
    textSettings = TEXT_SETTINGS_1;
  }
*/
  // Render tap if there is a change;
  for(int i = 0; i < 4; i++) {
    int currentPouringTap = this->currentPouringTaps[i];
    if (currentPouringTap < 0) {
      continue;
    }

    int x = TEXT_SETTINGS_4[i][0];
    int y = TEXT_SETTINGS_4[i][1];
    int fontSize = TEXT_SETTINGS_4[i][2];
    int offsetType = TEXT_SETTINGS_4[i][3];

    // If the tap stopped pouring, clean it up.
    if (!this->taps[currentPouringTap].IsPouring()) {
      changeCount++;
      this->display->ClearText(
        this->currentDisplays[i],
        x - this->GetXOffset(this->currentDisplays[i], offsetType, fontSize),
        y,
        fontSize
      );

      this->currentPouringTaps[i] = -1;
      this->currentDisplays[i] = "";

      continue;
    }

    uint pulses = this->taps[currentPouringTap].GetTotalPulses();
  	float ounces = (float)pulses * (float)128 / (float)10313;
    char ounceString[12];
  	sprintf(
      ounceString,
      "%.1f oz",
      ounces
    );

    if (String(ounceString) == this->currentDisplays[i]) {
      continue;
    }

    changeCount++;

    this->display->ClearText(
      this->currentDisplays[i],
      x - this->GetXOffset(this->currentDisplays[i], offsetType, fontSize),
      y,
      fontSize
    );

    this->currentDisplays[i] = ounceString;
    this->display->SetText(
      this->currentDisplays[i],
      x - this->GetXOffset(this->currentDisplays[i], offsetType, fontSize),
      y,
      fontSize
    );
  }

  return changeCount;
}

void PourDisplay::SetEmptySlotForTap(int tapId) {
  // For four taps, always render in the same spot.
  if (this->tapCount == 4) {
    this->currentPouringTaps[tapId] = tapId;
    return;
  }

  int minPouringTaps = min(this->tapCount, 4);

  // Find an empty slot for the tap if one hasn't already been set for it.
  for(int i = 0; i < minPouringTaps; i++) {
    int currentPouringTap = this->currentPouringTaps[i];
    // Kick out if already set
    if (currentPouringTap == tapId) {
      break;
    }

    // Continue if the spot is already occupied
    if (currentPouringTap >= 0) {
      continue;
    }

    this->currentPouringTaps[i] = tapId;
    break;
  }
}

int PourDisplay::GetXOffset(String text, int offsetType, int fontSize) {
  if (offsetType == 0) {
    return 0;
  }

  int offset = text.length() * 6 * fontSize;
  if (offsetType == 2) {
    return (int)round(offsetType / 2.0);
  }

  return offset;
}
