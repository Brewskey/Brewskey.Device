#ifndef TapConstraint_h
#define TapConstraint_h

#include "TapConstraintType.h"
#include "application.h"

struct TapConstraint {
  uint32_t tapIndex = 0;
  uint8_t type = TapConstraintType::NONE;
  uint32_t pulses = 0;
};

#endif
