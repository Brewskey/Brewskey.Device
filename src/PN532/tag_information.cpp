#include "tag_information.h"


// 4 byte id
//  - ATQA 0x4 && SAK 0x8 - Mifare Classic
//  - ATQA 0x304 && SAK 0x20 DESFire EV1 NUID (random)
// 7 byte id
//  - ATQA 0x44 && SAK 0x8 - Mifare Classic
//  - ATQA 0x44 && SAK 0x0 - Mifare Ultralight NFC Forum Type 2
//  - ATQA 0x344 && SAK 0x20 - NFC Forum Type 4 - DESFire  EV1
TagInformation::type TagInformation::GetTagType()
{
  if (this->uidLength == 4) {
    switch (this->sens_res) {
      case 0x04: {
        if (this->sak != 0x08) {
          return TagInformation::UNKNOWN;
        }

        return TagInformation::MIFARE_CLASSIC;
      }

      case 0x304: {
        if (this->sak != 0x20) {
          return TagInformation::UNKNOWN;
        }

        return TagInformation::DESFIRE_EV1_RANDOM;
      }
    }
  } else if (this->uidLength == 7) {
    switch (this->sens_res) {
      case 0x44: {
        if (this->sak != 0x00) {
          return TagInformation::UNKNOWN;
        }

        return TagInformation::MIFARE_ULTRALIGHT;
      }

      case 0x344: {
        if (this->sak != 0x20) {
          return TagInformation::UNKNOWN;
        }

        return TagInformation::DESFIRE_EV1;
      }
    }
  }
  
  return TagInformation::UNKNOWN;
}
