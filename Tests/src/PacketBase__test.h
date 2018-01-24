#pragma once

#include "Tappt/Packets/PacketBase.h"

class PacketTestImpl : public PacketBase {
public:
  PacketTestImpl() : PacketBase(1, 0x01) {}
  PacketTestImpl(uint8_t dataSize) : PacketBase(dataSize, 0x01) {}

  void SetDestination(uint8_t destination) {
    PacketBase::SetDestination(destination);
  }

  virtual void PrepareDataPacket() {
    PacketBase::PrepareDataPacket();
  }

  uint8_t* GetDataArray() {
    return this->dataPacket;
  }
  
  virtual void ResetDataPacket() {
  }
};

TEST_CASE("PacketBase::SetDestination", "[PacketBase]") {
  SECTION("Destination address is set correctly") {
    PacketTestImpl packet;
    uint8_t destination = 0xFF;
    packet.SetDestination(destination);
    REQUIRE(packet.GetDataArray()[DESTINATION_INDEX] == destination);
  }
};

TEST_CASE("PacketBase::PrepareDataPacket", "[PacketBase]") {
  SECTION("Calculates the correct checksum") {
    PacketTestImpl packet;
    uint8_t* data = packet.GetDataArray();
    packet.PrepareDataPacket();
    uint8_t checksum = data[4];
    REQUIRE(checksum == 51);
  }

  SECTION("Calculates the correct checksum with data") {
    PacketTestImpl packet = PacketTestImpl(2);
    uint8_t* data = packet.GetDataArray();
    data[3] = 0xEE;
    data[4] = 0xEE;
    packet.PrepareDataPacket();
    uint8_t checksum = data[5];
    REQUIRE(checksum == 254);
  }
  /*
  SECTION("tap ID is equal") {
    Tap tap;
    tap.Setup(&mock.get(), 123, 0);
    REQUIRE(tap.GetId() == 123);
  }*/
}
