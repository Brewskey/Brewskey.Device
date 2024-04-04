#include "PacketBase.h"

#define PACKET_SOURCE 0x00 /*source - mainboard*/

// Extra bytes in the array used for identifying the packet
// and determining if it's valid.
#define NON_DATA_BYTES_SIZE 4  // includes checksum

PacketBase::PacketBase(uint8_t dataSize, uint8_t packetType) {
  this->packetSize = dataSize + NON_DATA_BYTES_SIZE;
  this->dataPacket = new uint8_t[this->packetSize];
  memset(this->dataPacket, 0x00, this->packetSize);

  this->SetDestination(0x00);  // Default - all devices
  this->dataPacket[PACKET_SOURCE_INDEX] = PACKET_SOURCE;
  this->dataPacket[PACKET_TYPE_INDEX] = packetType;
}

PacketBase::~PacketBase() { delete[] this->dataPacket; }

void PacketBase::SetDestination(uint8_t destination) {
  this->dataPacket[PACKET_DESTINATION_INDEX] = destination;
}

uint8_t PacketBase::GetDestination() {
  return this->dataPacket[PACKET_DESTINATION_INDEX];
}

void PacketBase::PrepareDataPacket() {
  uint8_t checksum = 0;
  uint8_t ii;

  const uint8_t checksumIndex = this->packetSize - 1;
  /*calculate checksum*/
  for (ii = 0; ii < checksumIndex; ii++) {
    checksum ^= dataPacket[ii];
  }

  dataPacket[checksumIndex] = 255 - checksum;
}

/*process and send packet*/
void PacketBase::Send() {
#ifdef EXPANSION_BOX_PIN
  /*set RS485 direction pin HIGH: transmitter*/
  digitalWrite(EXPANSION_BOX_PIN, HIGH);
#endif

  this->PrepareDataPacket();

  uint8_t* str = this->dataPacket;  //.. str = pstr
  int i = 0;

  /*packet needs to start with the start-of-packet byte (ASCII '+' value)*/
  Serial1.write('+');

  while (i < this->packetSize) /* Loop through the packet data bytes */
  {
    /*if any of the special bytes are found, escape them by sending ASCII '#'
      before the byte*/
    if (*str == '*' || *str == '+' || *str == '-' || *str == '#') {
      Serial1.write('#');
    }

    // Serial.print(*str, HEX);
    // Serial.print(" ");

    /*send data byte*/
    Serial1.write(*str);
    str++; /* Point to next char */
    i++;   /* Incr length index */
  }

  /*packet needs to end with the end-of-packet byte (ASCII '-' value)*/
  Serial1.write('-');

  /*wait for serial data to be transfered*/
  Serial1.flush();
  // Serial.println();

  this->ResetDataPacket();

#ifdef EXPANSION_BOX_PIN
  /*set RS485 direction pin LOW: receiver*/
  digitalWrite(EXPANSION_BOX_PIN, LOW);
#endif
}
