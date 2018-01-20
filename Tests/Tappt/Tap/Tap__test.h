#include "Tappt/Tap/Tap.h"
#include "Tappt/KegeratorState/IStateManager.h"

fakeit::Mock<IStateManager> mock;
TEST_CASE("Tap", "[GetId]") {
	SECTION("tap ID is empty") {
		Tap tap;
		REQUIRE(tap.GetId() == String());
	}

	SECTION("tap ID is not empty") {
		Tap tap;
		tap.Setup(&mock.get(), "test", 0);
		REQUIRE(tap.GetId() == "test");
	}
}
