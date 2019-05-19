#pragma once

#include "Tappt/KegeratorStateMachine/KegeratorStateMachine.h"

TEST_CASE("KegeratorStateMachine::StartPour", "[KegeratorStateMachine]") {
  SECTION("zero constraints") {
    fakeit::Mock<Display> display;
    fakeit::Mock<NfcClient> nfcClient;
    fakeit::Mock<Sensors> sensors;
    When(Method(sensors, SetState)).AlwaysReturn();
    When(Method(sensors, OpenSolenoids)).AlwaysReturn();
    When(Method(nfcClient, Setup)).AlwaysReturn();

    KegeratorStateMachine sm(&display.get(), &nfcClient.get(), &sensors.get());
    sm.StartPour("token", 0, NULL);
    Verify(Method(sensors, OpenSolenoids)).Once();
  }

  SECTION("one constraints") {
    fakeit::Mock<Display> display;
    fakeit::Mock<NfcClient> nfcClient;
    fakeit::Mock<Sensors> sensors;
    When(Method(sensors, SetState)).AlwaysReturn();
    When(Method(sensors, OpenSolenoid)).AlwaysReturn();
    When(Method(nfcClient, Setup)).AlwaysReturn();

    KegeratorStateMachine sm(&display.get(), &nfcClient.get(), &sensors.get());

    DeviceSettings settings;
    settings.tapCount = 2;
    uint32_t tapIds[2] = { 1,2 };
    uint32_t pulsesPerGallon[2] = { 100, 200 };
    settings.tapIds = tapIds;
    settings.pulsesPerGallon = pulsesPerGallon;
    sm.Initialize(&settings);

    TapConstraint constraints[1];
    constraints[0].tapIndex = 1;
    constraints[0].type = TapConstraintType::MAXIMUM_VOLUME;
    constraints[0].pulses = 100;

    sm.StartPour("token", 1, constraints);
    Verify(Method(sensors, OpenSolenoid)).Once();
  }
}
