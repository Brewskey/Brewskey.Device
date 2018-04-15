#pragma once

#include "Tappt/Sensors/Sensors.h"

TEST_CASE("Sensors::ReadMultitap", "[Sensors]") {
  IKegeratorStateMachine &ks = Mock<IKegeratorStateMachine>().get();

  SECTION("Read packet is not ready") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, Read)).AlwaysReturn();
    When(Method(mock, IsPacketReady)).AlwaysReturn(false);
    Sensors sensors = Sensors(mock.get());
    Tap *taps = NULL;
    sensors.Setup(&ks, taps, 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Never();
  }

  SECTION("Read packet is not valid") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(false);
    Sensors sensors = Sensors(mock.get());
    Tap *taps = NULL;
    sensors.Setup(&ks, taps, 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).Never();
    Verify(Method(mock, GetDestination)).Never();
    Verify(Method(mock, GetSource)).Never();
  }

  SECTION("Destination not zero") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(POUR_PACKET_TYPE);
    When(Method(mock, GetDestination)).AlwaysReturn(0xFF);
    When(Method(mock, GetSource)).AlwaysReturn(0x01);
    Sensors sensors = Sensors(mock.get());
    Tap *taps = NULL;
    sensors.Setup(&ks, taps, 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).Once();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Never();
  }

  SECTION("Packet type not pour packet") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(0);
    When(Method(mock, GetDestination)).AlwaysReturn(0x00);
    When(Method(mock, GetSource)).AlwaysReturn(0x01);
    Sensors sensors = Sensors(mock.get());
    Tap *taps = NULL;
    sensors.Setup(&ks, taps, 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).Once();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Never();
  }

  SECTION("Source not 0x01") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(POUR_PACKET_TYPE);
    When(Method(mock, GetDestination)).AlwaysReturn(0x00);
    When(Method(mock, GetSource)).AlwaysReturn(0x00);
    Sensors sensors = Sensors(mock.get());
    sensors.Setup(&ks, NULL, 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).Once();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Never();
  }


  SECTION("Empty packet buffer") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(POUR_PACKET_TYPE);
    When(Method(mock, GetDestination)).AlwaysReturn(0x00);
    When(Method(mock, GetPacketType)).AlwaysReturn(0x01);
    When(Method(mock, GetSource)).AlwaysReturn(0x01);

    uint8_t data[17]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    When(Method(mock, GetDataBuffer)).AlwaysReturn(data);

    Sensors sensors = Sensors(mock.get());
    Mock<Tap> tapMock;
    sensors.Setup(&ks, &tapMock.get(), 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).AtLeastOnce();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetPacketType)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Once();
    Verify(Method(tapMock, GetTotalPulses)).Never();
  }

  SECTION("Single tap equal pulses") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(POUR_PACKET_TYPE);
    When(Method(mock, GetDestination)).AlwaysReturn(0x00);
    When(Method(mock, GetSource)).AlwaysReturn(0x01);

    uint8_t data[17]{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    When(Method(mock, GetDataBuffer)).AlwaysReturn(data);

    Sensors sensors = Sensors(mock.get());

    Mock<Tap> tapMock;
    When(Method(tapMock, GetTotalPulses)).AlwaysReturn(1);
    // When(Method(tapMock, AddToFlowCount)).AlwaysReturn(1);
    sensors.Setup(&ks, &tapMock.get(), 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).AtLeastOnce();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Once();
    Verify(Method(tapMock, SetTotalPulses)).Never();
  }

  SECTION("Single tap new pulses") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(POUR_PACKET_TYPE);
    When(Method(mock, GetDestination)).AlwaysReturn(0x00);
    When(Method(mock, GetSource)).AlwaysReturn(0x01);

    uint8_t data[17]{ 0, 0, 0, 0, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    When(Method(mock, GetDataBuffer)).AlwaysReturn(data);

    Sensors sensors = Sensors(mock.get());

    Mock<Tap> tapMock;
    When(Method(tapMock, GetTotalPulses)).AlwaysReturn(0);
    When(Method(tapMock, SetTotalPulses)).AlwaysReturn();
    sensors.Setup(&ks, &tapMock.get(), 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).AtLeastOnce();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Once();
    Verify(Method(tapMock, SetTotalPulses).Using(0xFF)).Once();
  }

  SECTION("Single tap pulses from slot 2") {
    PacketReader cls;
    fakeit::Mock<PacketReader> mock(cls);
    When(Method(mock, IsPacketReady)).AlwaysReturn(true);
    When(Method(mock, IsValid)).AlwaysReturn(true);
    When(Method(mock, GetPacketType)).AlwaysReturn(POUR_PACKET_TYPE);
    When(Method(mock, GetDestination)).AlwaysReturn(0x00);
    When(Method(mock, GetSource)).AlwaysReturn(0x01);

    uint8_t data[17]{ 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0 };
    When(Method(mock, GetDataBuffer)).AlwaysReturn(data);

    Sensors sensors = Sensors(mock.get());

    Mock<Tap> tapMock;
    When(Method(tapMock, GetTotalPulses)).AlwaysReturn(0);
    When(Method(tapMock, SetTotalPulses)).AlwaysReturn();
    sensors.Setup(&ks, &tapMock.get(), 1);
    sensors.Tick();
    sensors.ReadMultitap();
    Verify(Method(mock, IsValid)).Once();
    Verify(Method(mock, GetPacketType)).AtLeastOnce();
    Verify(Method(mock, GetDestination)).Once();
    Verify(Method(mock, GetSource)).Once();
    Verify(Method(mock, GetDataBuffer)).Once();
    Verify(Method(tapMock, SetTotalPulses)).Never();
  }
};