#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "amcom.h"
#include "amcom_packets.h"


#define PLAYER_NAME "White power."
#define PLAYER_MSG "bitches come and go."
#define PLAYER_GONE "bullshit."


float force_move = 12.0;

void AMCOM_Print(AMCOM_ObjectState * current_object, uint8_t objects_amount){
   
    const char* types[4] = {"PLAYA","TRANSISTOR", "SPARK", "GLUE" };

    for (uint8_t i = 0; i< objects_amount; ++i){
        
        // set next object
        printf("-------------------- %u -------------------\n",current_object[i].objectNo);
        printf("O.type:   %s \n",types[current_object[i].objectType] );
        printf("O.number: %u \n",current_object[i].objectNo );
        printf("O.HP:     %d \n",current_object[i].hp );
        printf("O.POSX:   %f \n",current_object[i].x );
        printf("O.POSY:   %f \n",current_object[i].y );
        printf("--------------------end-------------------\n");  
    }
}

void PrintObject(AMCOM_ObjectState * o){
    if(o){
        printf("O.type:   %u \n",o->objectType );
        printf("O.number: %lu \n",o->objectNo );
        printf("O.HP:     %d \n",o->hp );
        printf("O.POSX:   %f \n",o->x );
        printf("O.POSY:   %f \n",o->y );
    }
    else{
        printf("ERROR! not reading the NULL, dont be stupid\n");
    }
}

typedef struct node{
    AMCOM_ObjectState * data;
    struct node * next;
}ll_t;



ll_t * createNode(AMCOM_ObjectState * o){
    ll_t* node = (ll_t* )malloc(sizeof(ll_t));
    node->data = o;
    node->next = NULL;
    return node;
}
void pushNode(ll_t** head, AMCOM_ObjectState * o ){
    
    ll_t* node = createNode(o);

    node->next = *head;
    *head = node;
    
}

void printNode(ll_t** head){
    ll_t* current = *head;
    while(current != NULL){

        PrintObject(current->data);
        current = current->next;
        
    }
   
}

void clearList(ll_t ** head){
    ll_t* current =  *head;
    ll_t* next;
    while(current != NULL){
        next = current->next;
        free(current);
        current=next;
    }    
    *head = NULL;
}

ll_t* head_p = NULL;
ll_t* head_t = NULL;
ll_t* head_s = NULL;
ll_t* head_g = NULL;


typedef struct GameDetails{
    uint8_t total_players;
    uint8_t my_player;
    uint8_t current_angle;

}GameDetails_t;


GameDetails_t * GameDetails = NULL;
void GameDetailsInit(void){
    GameDetails->current_angle = 0;
    GameDetails->my_player = 0;
    GameDetails->total_players = 0;
}




uint16_t findObject( uint8_t type, uint16_t num){
    ll_t* p = NULL;
    uint16_t count = 0;

    switch(type){
        case 0: p = head_p; break;
        case 1: p = head_t; break;
        case 2: p = head_s; break;
        case 3: p = head_g; break;
    }
    
    while(p != NULL){
        
        if( p->data->objectType == type &&  p->data->objectNo == num){
            return count;
        }
        p = p->next;
        count++;
    }
    return 255;
}

uint16_t findClosest(uint8_t typeOrigin, uint8_t typeDest, uint16_t numOrigin, uint16_t numDest){
    
}

void fetchObjects(AMCOM_ObjectState * state, uint8_t count){
    for(uint8_t i = 0; i< count; ++i){
        ll_t* p = NULL;
        switch(state[i].objectType){
            case 0: p = head_p; pushNode(&head_p, &state[i]); break;
            case 1: p = head_t; pushNode(&head_t, &state[i]); break;
            case 2: p = head_s; pushNode(&head_s, &state[i]); break;
            case 3: p = head_g; pushNode(&head_g, &state[i]); break;
        }
        
        // printNode(&p);   
    }
}





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
            AMCOM_NewGameResponsePayload newGameResponse;
            sprintf(newGameResponse.helloMessage, PLAYER_MSG);

            
            // uint8_t objects_amount = packet->header.length / (uint8_t)sizeof(AMCOM_ObjectState);//  packet size-> 12
            AMCOM_NewGameRequestPayload * ptr = (AMCOM_NewGameRequestPayload  *)packet->payload;
            
            GameDetails->my_player = ptr->playerNumber;
            GameDetails->total_players = ptr->numberOfPlayers;


            toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &newGameResponse, sizeof(newGameResponse), buf);
            break;
        
        case AMCOM_GAME_OVER_REQUEST:
            printf("GAME_OVER.request. Responding with %s.\n", PLAYER_GONE);
            AMCOM_GameOverResponsePayload gameOverResponse;
            sprintf(gameOverResponse.endMessage, PLAYER_GONE);
            
            
            clearList(&head_p);
            clearList(&head_t);
            clearList(&head_s);
            clearList(&head_g);

            
            
            toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &gameOverResponse, sizeof(gameOverResponse), buf);
            break;
        

        case AMCOM_OBJECT_UPDATE_REQUEST:
            printf("OBJECTS.request. Reading the data\n");

            uint8_t objects_amount = packet->header.length / (uint8_t)sizeof(AMCOM_ObjectState);//  packet size-> 12
            AMCOM_ObjectState * current_object = (AMCOM_ObjectState *)packet->payload;
            
            fetchObjects(current_object, objects_amount);

            // uint16_t x = 255;
            // if( (x = findObject( (uint8_t)1,(uint16_t)5)) != (uint16_t)255){
            //     printf("\nZnaleziony!: %u\n",x);
            // }else{
            //     printf("\nNie naleziony!: %u\n",x);
                
            }
            // AMCOM_Print(current_object, objects_amount);

            break;  

        case AMCOM_MOVE_REQUEST:
            printf("MOVE.request. Responding with MOVING\n");

            AMCOM_MoveResponsePayload moveResponse;
            force_move += 0.1;

            moveResponse.angle = force_move;
            
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

    GameDetails = (GameDetails_t*)malloc(sizeof(GameDetails_t));
    GameDetailsInit();





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
    closesocket(ConnectSocket);
    // Clean up
    WSACleanup();

    return 0;
}


