#include "PacketReader.h"

uint8_t PacketReader::GetDestination() {
  return this->incomingBuffer[PACKET_DESTINATION_INDEX];
}

uint8_t PacketReader::GetSource() {
  return this->incomingBuffer[PACKET_SOURCE_INDEX];
}

uint8_t PacketReader::GetPacketType() {
  return this->incomingBuffer[PACKET_TYPE_INDEX];
}

uint8_t PacketReader::GetBufferSizeForPacket() {
  switch (this->GetPacketType()) {
    case POUR_PACKET_TYPE: {
      return INCOMING_POUR_BUFFER_SIZE;
    }

    case CONFIGURATION_RESPONSE_PACKET_TYPE: {
      return CONFIGURATION_RESPONSE_BUFFER_SIZE;
    }
  }

  return 0;
}

uint8_t* PacketReader::GetDataBuffer() {
  return (uint8_t*)(this->incomingBuffer + DATA_START_INDEX);
}

uint8_t PacketReader::GetDataBufferSize() {
  return this->GetBufferSizeForPacket() - (DATA_START_INDEX + 1);
}

bool PacketReader::IsPacketReady() { return this->isPacketReady; }

bool PacketReader::IsValid() { return this->isValid; }

void PacketReader::Reset() {
  this->isPacketReady = false;
  this->isValid = false;
  this->count = 0; /* Reset Counter - since start of packet */

  memset(this->incomingBuffer, 0, sizeof(this->incomingBuffer));
}

void PacketReader::Read() {
  uint8_t ii, checksum = 0, data = 0;

  /*read all received bytes*/
  while (Serial1.available() > 0) {
    data = Serial1.read();              /* Get data */
    if (data == '#' && !this->esc_flag) /* If finding first escape byte */
    {
      this->esc_flag = 1; /* Set escape byte flag */
    } else {
      /* Escape byte not set */
      if (!this->esc_flag) {
        /* Getting sync byte of packet, since no escape byte before it */
        if (data == '+') {
          this->Reset();
          for (ii = 0; ii < PACKET_BUFFER; ii++) {
            this->incomingBuffer[ii] = 0; /* Clearing packet buffer */
          }

          continue;
        }

        if (data == '-') /* End of packet */
        {
          checksum = 0; /* Reset checksum */

          for (ii = 0; ii < this->count - 1;
               ii++) /* Calculating checksum of packet */
          {
            checksum ^= this->incomingBuffer[ii];
          }

          checksum = 255 - checksum;

          if (checksum == this->incomingBuffer[count - 1]) {
            this->isValid = 1; /*packet is valid*/
          } else {
            this->isValid = 0; /* packet invalid */
          }

          this->isPacketReady = true;
          break;
        }
      } else {
        this->esc_flag = 0;
      }

      /* If count still less than packet buffer size */
      if (this->count < PACKET_BUFFER) {
        this->incomingBuffer[count] = data; /* Store data in buffer */
        this->count++;                      /* Increment counter */
      }
    }
  }
}
