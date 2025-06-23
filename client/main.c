#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// USER ADD
// END USER ADD

#include "amcom.h"
#include "amcom_packets.h"
// USER ADD

#include "game_details.h"
#include "objects.h"
#include "node.h"
#include "alg.h"

// END USER ADD


#define PLAYER_NAME "White power."
#define PLAYER_MSG "bitches come and go."
#define PLAYER_GONE "bullshit."


/*
    global register of game details, made global for the sole purpose of not messing up with already-established
    tcp connectors and other pre-made funcionalities 

*/
GameDetails_t *  GameDetails = NULL; 




// na podstawie tego ile ich jest, dodaj je do listy



   
void amPacketHandler(const AMCOM_Packet* packet, void* userContext) {

    uint8_t buf[AMCOM_MAX_PACKET_SIZE];              // buffer used to serialize outgoing packets
    size_t toSend = 0;                               // size of the outgoing packet
    SOCKET ConnectSocket  = *((SOCKET*)userContext); // socket used for communication with the server

    switch (packet->header.type) {
        case AMCOM_IDENTIFY_REQUEST:

            printf("IDENTIFY.request. Responding with %s.\n", PLAYER_NAME);
            AMCOM_IdentifyResponsePayload identifyResponse;
            sprintf(identifyResponse.playerName, PLAYER_NAME);

            toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
            break;
    
        case AMCOM_NEW_GAME_REQUEST:

            printf("NEW_GAME.request. Responding with %s.\n", PLAYER_MSG);
            GameDetailsInit(GameDetails);

            AMCOM_NewGameRequestPayload * gameData =(AMCOM_NewGameRequestPayload *)packet->payload ;
            if (GameDetails->objects == 0) FillGamedata(gameData); // fill if 

          
            // send player name
            AMCOM_NewGameResponsePayload newGameResponse;
            sprintf(newGameResponse.helloMessage, PLAYER_MSG);
            toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &newGameResponse, sizeof(newGameResponse), buf);
            break;
        
        case AMCOM_GAME_OVER_REQUEST:

            
            // clear the nodes after the game has finished
            // deleteAllNodes(4,&GameDetails->head_p, &GameDetails->head_t,&GameDetails->head_s,&GameDetails->head_g);
            deleteAllNode(&GameDetails->head_p);
            deleteAllNode(&GameDetails->head_t);
            deleteAllNode(&GameDetails->head_s);
            deleteAllNode(&GameDetails->head_g);


            printf("GAME_OVER.request. Responding with %s.\n", PLAYER_GONE);
            AMCOM_GameOverResponsePayload gameOverResponse;
            sprintf(gameOverResponse.endMessage, PLAYER_GONE);


            toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &gameOverResponse, sizeof(gameOverResponse), buf);
            break;
        

        case AMCOM_OBJECT_UPDATE_REQUEST:
            printf("OBJECTS.request. Reading the data\n");



            uint8_t objects_amount = packet->header.length / (uint8_t)sizeof(AMCOM_ObjectState);
            AMCOM_ObjectState * current_objects = (AMCOM_ObjectState *)packet->payload;
            
        
               

            OBJ_FillObjects(current_objects, objects_amount);
                            
            
            // OBJ_PrintStacks(4,, &GameDetails->head_t,&GameDetails->head_s,&GameDetails->head_g );
            printStack(&GameDetails->head_p);
            printStack(&GameDetails->head_t);
            printStack(&GameDetails->head_s);
            printStack(&GameDetails->head_g);
        
            
            break;  

        case AMCOM_MOVE_REQUEST:
            printf("MOVE.request. Responding with MOVING\n");


            AMCOM_MoveResponsePayload moveResponse;


            // force_move += 0.1;

            // go after the closest one 
            // 
            
            
            moveResponse.angle = 1.0f;
            
            toSend = AMCOM_Serialize(AMCOM_MOVE_RESPONSE, &moveResponse, sizeof(moveResponse), buf);
            break;

      

    }

	// if there is something to send back - do it
	if (toSend > 0) {
		int bytesSent = send(ConnectSocket, (const char*)buf, toSend, 0 );
		if (bytesSent == SOCKET_ERROR) {
			printf("Socket send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			return;
		}
	}
}


#define GAME_SERVER "localhost"
#define GAME_SERVER_PORT "2001"

int main(int argc, char **argv) {

    GameDetails = (GameDetails_t * )malloc(sizeof(GameDetails_t));
    GameDetailsInit(GameDetails);


    printf("This is mniAM player. Let's eat some transistors! \n");

    WSADATA wsaData;
    int iResult;

    // Initialize Winsock library (windows sockets)
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Prepare temporary data
    SOCKET ConnectSocket  = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;
    int iSendResult;
    char recvbuf[512];
    int recvbuflen = sizeof(recvbuf);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the game server address and port
    iResult = getaddrinfo(GAME_SERVER, GAME_SERVER_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    printf("Connecting to game server...\n");
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    // Free some used resources
    freeaddrinfo(result);

    // Check if we connected to the game server
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to the game server!\n");
        WSACleanup();
        return 1;
    } else {
        printf("Connected to game server\n");
    }

   

    AMCOM_Receiver amReceiver;
    AMCOM_InitReceiver(&amReceiver, amPacketHandler, &ConnectSocket);
    
    

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            AMCOM_Deserialize(&amReceiver, recvbuf, iResult);
        } else if ( iResult == 0 ) {
            printf("Connection closed\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }

    } while( iResult > 0 );

    // No longer need the socket
 
    
    free(GameDetails);
    closesocket(ConnectSocket);
    // Clean up
    WSACleanup();


    return 0;
}


