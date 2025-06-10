#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "amcom.h"

/// Start of packet character
const uint8_t  AMCOM_SOP         = 0xA1;
const uint16_t AMCOM_INITIAL_CRC = 0xFFFF;

static uint16_t AMCOM_UpdateCRC(uint8_t byte, uint16_t crc)
{
	byte ^= (uint8_t)(crc & 0x00ff);
	byte ^= (uint8_t)(byte << 4);
	return ((((uint16_t)byte << 8) | (uint8_t)(crc >> 8)) ^ (uint8_t)(byte >> 4) ^ ((uint16_t)byte << 3));
}


void AMCOM_InitReceiver(AMCOM_Receiver* receiver, AMCOM_PacketHandler packetHandlerCallback, void* userContext) {
    if (!receiver || !packetHandlerCallback) {
        return;
    }

    // Initialize the receiver structure
    receiver->packetHandler = packetHandlerCallback;
    receiver->userContext = userContext;
    receiver->payloadCounter = 0;
    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
    
    // Clear the received packet buffer
    memset(&receiver->receivedPacket, 0, sizeof(AMCOM_Packet));
}

size_t AMCOM_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {
	// TODO
	// Validate input arguments
    if (!destinationBuffer || (payloadSize > 0 && !payload) || payloadSize > AMCOM_MAX_PAYLOAD_SIZE) {
        return 0; // Invalid input
    }

    // Initialize the packet header
    AMCOM_PacketHeader header;
    header.sop = AMCOM_SOP; // Start of Packet
    header.type = packetType;
    header.length = (uint8_t)payloadSize;

    // Calculate CRC for TYPE, LENGTH, and PAYLOAD
    uint16_t crc = AMCOM_INITIAL_CRC;
    crc = AMCOM_UpdateCRC(header.type, crc);
    crc = AMCOM_UpdateCRC(header.length, crc);

    if (payload && payloadSize > 0) {
        const uint8_t* payloadBytes = (const uint8_t*)payload;
        for (size_t i = 0; i < payloadSize; i++) {
            crc = AMCOM_UpdateCRC(payloadBytes[i], crc);
        }
    }
    header.crc = crc;

    // Serialize the header into the destination buffer
    size_t index = 0;
    destinationBuffer[index++] = header.sop;
    destinationBuffer[index++] = header.type;
    destinationBuffer[index++] = header.length;
    destinationBuffer[index++] = (uint8_t)(header.crc & 0xFF);       // CRC low byte (little-endian)
    destinationBuffer[index++] = (uint8_t)((header.crc >> 8) & 0xFF); // CRC high byte (little-endian)

    // Serialize the payload into the destination buffer
    if (payload && payloadSize > 0) {
        memcpy(&destinationBuffer[index], payload, payloadSize);
        index += payloadSize;
    }

    // Return the total size of the serialized packet
    return index;
}

void AMCOM_Deserialize(AMCOM_Receiver* receiver, const void* data, size_t dataSize) {
    if (!receiver || !data || dataSize == 0) {
        return;
    }

    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < dataSize; i++) {
        uint8_t byte = bytes[i];

        switch (receiver->receivedPacketState) {
            case AMCOM_PACKET_STATE_EMPTY:
                if (byte == AMCOM_SOP) {
                    receiver->receivedPacket.header.sop = byte;
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_SOP;
                }
                break;

            case AMCOM_PACKET_STATE_GOT_SOP:
                receiver->receivedPacket.header.type = byte;
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_TYPE;
                break;

            case AMCOM_PACKET_STATE_GOT_TYPE:
                if (byte <= AMCOM_MAX_PAYLOAD_SIZE) {
                    receiver->receivedPacket.header.length = byte;
                    receiver->payloadCounter = 0;
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_LENGTH;
                } else {
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                }
                break;

            case AMCOM_PACKET_STATE_GOT_LENGTH:
                receiver->receivedPacket.header.crc = byte; // LSB first
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_CRC_LO;
                break;

            case AMCOM_PACKET_STATE_GOT_CRC_LO:
                receiver->receivedPacket.header.crc |= ((uint16_t)byte << 8); // MSB second
                
                if (receiver->receivedPacket.header.length == 0) {
                    // Calculate CRC for packet without payload
                    uint16_t calculatedCRC = AMCOM_INITIAL_CRC;
                    calculatedCRC = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, calculatedCRC);
                    calculatedCRC = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, calculatedCRC);
                    
                    if (calculatedCRC == receiver->receivedPacket.header.crc) {
                        receiver->packetHandler(&receiver->receivedPacket, receiver->userContext);
                    }
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                } else {
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GETTING_PAYLOAD;
                }
                break;

            case AMCOM_PACKET_STATE_GETTING_PAYLOAD:
                receiver->receivedPacket.payload[receiver->payloadCounter++] = byte;
                
                if (receiver->payloadCounter >= receiver->receivedPacket.header.length) {
                    // Calculate CRC for packet with payload
                    uint16_t calculatedCRC = AMCOM_INITIAL_CRC;
                    calculatedCRC = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, calculatedCRC);
                    calculatedCRC = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, calculatedCRC);
                    
                    for (size_t j = 0; j < receiver->receivedPacket.header.length; j++) {
                        calculatedCRC = AMCOM_UpdateCRC(receiver->receivedPacket.payload[j], calculatedCRC);
                    }
                    
                    if (calculatedCRC == receiver->receivedPacket.header.crc) {
                        receiver->packetHandler(&receiver->receivedPacket, receiver->userContext);
                    }
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                }
                break;

            default:
                receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                break;
        }
    }
}


