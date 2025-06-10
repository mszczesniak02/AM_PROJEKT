#include "cmsis_os2.h"
#include "game.h"
#include "amcom.h"
#include "amcom_packets.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>

/**
 * This function will be called each time a valid AMCOM packet is received
 */
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext) {
	static uint8_t buf[AMCOM_MAX_PACKET_SIZE];  // buffer used to serialize outgoing packets
	size_t toSend = 0;                          // size of the outgoing packet
	Socket_t socket = (Socket_t)userContext;    // socket used for communication with the server

	switch (packet->header.type) {
	case AMCOM_IDENTIFY_REQUEST:
		printf("Got IDENTIFY.request. Responding with IDENTIFY.response\n");
		AMCOM_IdentifyResponsePayload identifyResponse;
		sprintf(identifyResponse.playerName, "STM32F4");
		toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
		break;
	default:
		break;
	}

	// if there is something to send back - do it
	if (toSend > 0) {
		FreeRTOS_send(socket, buf, toSend, 0);
	}
}
