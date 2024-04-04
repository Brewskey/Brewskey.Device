#pragma once

class KegeratorState {
 public:
  enum e {
    INITIALIZING = 0,
    LISTENING,
    WAITING_FOR_POUR_RESPONSE,
    POUR_AUTHORIZED,
    POURING,

    CLEANING,
    UNLOCKED,
    INACTIVE,
    CONFIGURE,
  };
};
