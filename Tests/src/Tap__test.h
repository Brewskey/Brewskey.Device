#pragma once

#include "Tappt/Tap/Tap.h"
#include "Tappt/KegeratorState/IStateManager.h"

TEST_CASE("Tap", "[GetId]") {
  fakeit::Mock<IStateManager> mock;
  SECTION("tap ID is empty") {
    Tap tap;
    REQUIRE(tap.GetId() == 0);
  }

  SECTION("tap ID is equal") {
    Tap tap;
    tap.Setup(&mock.get(), 123, 0);
    REQUIRE(tap.GetId() == 123);
  }
}
