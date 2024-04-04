#pragma once

#include "Tappt/Packets/PacketDefinitions.h"
#include "Tappt/Pins.h"
#include "application.h"

/*maximum size of incoming packet from ext board*/
#define PACKET_BUFFER 64

#if USE_BETA_PACKET_FORMAT == 1
#define INCOMING_POUR_BUFFER_SIZE 22
#define DATA_START_INDEX 4
#else
#define INCOMING_POUR_BUFFER_SIZE 21
#define DATA_START_INDEX 3
#endif

#define CONFIGURATION_RESPONSE_BUFFER_SIZE 8

class PacketReader {
 public:
  void Read();
  virtual uint8_t GetPacketType();
  virtual uint8_t GetDestination();
  virtual uint8_t GetSource();
  virtual uint8_t* GetDataBuffer();
  uint8_t GetDataBufferSize();
  virtual bool IsPacketReady();
  virtual bool IsValid();
  void Reset();

 private:
  uint8_t GetBufferSizeForPacket();
  uint8_t count = 0;
  uint8_t esc_flag = 0;
  bool isPacketReady = false;
  bool isValid = false;

  /*array for incoming packet*/
  uint8_t incomingBuffer[PACKET_BUFFER];
};
