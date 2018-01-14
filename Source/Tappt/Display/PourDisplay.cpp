#include "PourDisplay.h"

#define MAX_DRAWING_SLOTS 4
#define TEXT_SETTING_LENGTH 4
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
int TEXT_SETTINGS_1[1][TEXT_SETTING_LENGTH] = {
  {42, 40, 2, 0},
};

int TEXT_SETTINGS_4[MAX_DRAWING_SLOTS][TEXT_SETTING_LENGTH] = {
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

  for(int ii = 0; ii < this->tapCount; ii++) {
    if (this->taps[ii].IsPouring()) {
      counter++;
      this->SetEmptySlotForTap(ii);
    }
	}

  // Count the number of taps that rendered last
  int oldCounter = 0;
  for(int ii = 0; ii < MAX_DRAWING_SLOTS; ii++) {
    if (this->currentPouringTaps[ii] >= 0) {
      oldCounter++;
    }
  }

  int (*textSettings)[TEXT_SETTING_LENGTH] = TEXT_SETTINGS_4;
  if (counter == 1) {
    textSettings = TEXT_SETTINGS_1;
  }


  int (*oldTextSettings)[TEXT_SETTING_LENGTH] = TEXT_SETTINGS_4;
  if (oldCounter == 1) {
    oldTextSettings = TEXT_SETTINGS_1;
  }

  // Render tap if there is a change;
  for(int ii = 0; ii < MAX_DRAWING_SLOTS; ii++) {
    int currentPouringTap = this->currentPouringTaps[ii];
    if (
      currentPouringTap < 0 ||
      this->taps[currentPouringTap].GetPulsesPerGallon() < 1
    ) {
      continue;
    }

    int x = oldTextSettings[ii][0];
    int y = oldTextSettings[ii][1];
    int fontSize = oldTextSettings[ii][2];
    int offsetType = oldTextSettings[ii][3];

    // If the tap stopped pouring, clean it up.
    if (
      !this->taps[currentPouringTap].IsPouring() &&
      this->currentDisplays[ii].length() > 0
    ) {
      changeCount++;
      this->display->ClearText(
        this->currentDisplays[ii],
        x,
        y,
        fontSize,
        offsetType
      );

      this->currentPouringTaps[ii] = -1;
      this->currentDisplays[ii] = "";

      continue;
    }

    x = textSettings[ii][0];
    y = textSettings[ii][1];
    fontSize = textSettings[ii][2];
    offsetType = textSettings[ii][3];

    uint pulses = this->taps[currentPouringTap].GetTotalPulses();
    int pulsesPerGallon = this->taps[currentPouringTap].GetPulsesPerGallon();
  	float ounces = (float)pulses * (float)128 / (float)pulsesPerGallon;
    char ounceString[12];
  	sprintf(
      ounceString,
      ounces >= 100 && counter == 1 ? "%.0f oz" : "%.1f oz",
      ounces
    );

    if (String(ounceString) == this->currentDisplays[ii]) {
      continue;
    }

    changeCount++;

    this->display->ClearText(
      this->currentDisplays[ii],
      x,
      y,
      fontSize,
      offsetType
    );

    this->currentDisplays[ii] = ounceString;
    this->display->SetText(
      this->currentDisplays[ii],
      x,
      y,
      fontSize,
      offsetType
    );
  }

  return changeCount;
}

void bubbleSort(int arr[], int size);

void PourDisplay::SetEmptySlotForTap(int tapId) {
  int minPouringTaps = min(this->tapCount, MAX_DRAWING_SLOTS);

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

  bubbleSort(this->currentPouringTaps, MAX_DRAWING_SLOTS);
}

void swapInt(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// A function to implement bubble sort
void bubbleSort(int arr[], int size)
{
  int ii, jj;
  bool swapped;
  for (ii = 0; ii < size - 1; ii++)
  {
    swapped = false;
    for (jj = 0; jj < size - ii - 1; jj++)
    {
      // move empty slots to back of the array
      int current = arr[jj] == -1 ? INT_MAX : arr[jj];
      int next = arr[jj + 1] == -1 ? INT_MAX : arr[jj + 1];
      if (current > next)
      {
        swapInt(&arr[jj], &arr[jj + 1]);
        swapped = true;
      }
    }

    if (swapped == false)
    {
      break;
    }
  }
}
