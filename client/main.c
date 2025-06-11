#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
// USER ADD
#include <math.h>
// USER END

#include "amcom.h"
#include "amcom_packets.h"


#define PLAYER_NAME "White power."
#define PLAYER_MSG "bitches come and go."
#define PLAYER_GONE "bullshit."





typedef struct {
    float x;
    float y;
} ALG_Positions_t;



typedef struct {
        uint8_t objects_init;
        uint8_t objects_registered;

        AMCOM_ObjectState objects[AMCOM_MAX_OBJECT_UPDATES];
        


        uint8_t objects_total;
        uint8_t players_total;
        uint8_t player_num;
        
}ALG_GameDetails_t;

static ALG_GameDetails_t *  ALG_GameDetails = NULL; // global register of game details, made global for the sole purpose of not messing up with already-established tcp connectors and other pre-made funcionalities 

/*
    struct of the object requires some way of storing the position on a map, and calculating the trejectory(angle)
*/

// USER FUNCS

void ALG_GameDetailsInit(){

    ALG_GameDetails->objects_registered     = 0;
    ALG_GameDetails->objects_init           = 0;
    ALG_GameDetails->objects_total          = 0;
    ALG_GameDetails->player_num             = 0;
    ALG_GameDetails->players_total          = 0;
    
    memset(ALG_GameDetails->objects, 0, sizeof(ALG_GameDetails->objects));
};

float ALG_GetDistance(AMCOM_ObjectState * object_origin,AMCOM_ObjectState * object_dest ){
    // calculate distance in straight line from origin to dest, 
    return sqrt(  
                (object_dest->x - object_origin->x) * (object_dest->x - object_origin->x) + // (x-x0)^2
                (object_dest->y - object_origin->y) * (object_dest->y - object_origin->y)   // (y-y0)^2
                );
}

float ALG_GetRelativeAngle(AMCOM_ObjectState * object_origin, AMCOM_ObjectState * object_dest){// angle relative to the current player angle
    float distance_x = object_dest->x - object_origin->x;
    float distance_y = object_dest->y - object_origin->y;

    float angle = (float)atan2(distance_y, distance_x);
    // angle  = (angle * 180.0f / 3.14f);

    
    //  cast into int to get range of 0-360deg, then add the decimal values.
    // float decimal = angle -  (int)angle; 
    
    // angle = ((int)angle + 360) % 360;

    return angle ;
}

void ALG_Transpose(AMCOM_ObjectState * object){ //transpose from -500/500 to 0/1000
    object->x += 500.0f;
    object->y += 500.0f;
}

void ALG_FillGamedata(AMCOM_NewGameRequestPayload * game_data){
    ALG_GameDetails->players_total = game_data->numberOfPlayers;
    ALG_GameDetails->player_num = game_data->playerNumber;
    ALG_GameDetails->objects_init = 1;
}

void ALG_FillObjects(AMCOM_ObjectState * object, uint8_t object_amount){
    for(uint8_t i = 0 ; i<object_amount; ++i){
        ALG_GameDetails->objects[i] = object[i];
        ALG_Transpose(&ALG_GameDetails->objects[i]);

    }
    ALG_GameDetails->objects_total = object_amount;
    ALG_GameDetails->objects_registered = 1;
}

void ALG_PrintObjects(void){
    char * types[]  = {"PLAYER","TRANSISTOR", "SPARK", "GLUE"};
        uint8_t type_num;

        for (uint8_t i = 0; i< ALG_GameDetails->objects_total ; ++i){
            


            type_num = ALG_GameDetails->objects[i].objectType;
            
            printf("\nO.number: %u \n",ALG_GameDetails->objects[i].objectNo );
            printf("O.type:   %s \n",types[type_num] );
            printf("O.HP:     %d \n",ALG_GameDetails->objects[i].hp );
            printf("O.POSX:   %f \n",ALG_GameDetails->objects[i].x );
            printf("O.POSY:   %f \n\n",ALG_GameDetails->objects[i].y );
            
        }
}

AMCOM_ObjectState * ALG_FindPlayer(void) {
    AMCOM_ObjectState * ptr = ALG_GameDetails->objects;
    
    for (uint8_t i = 0; i < ALG_GameDetails->objects_total; ++i) {
        if(ptr->objectType == 0) {  // Sprawdź czy to jest PLAYER (typ 0)
            return ptr;
        }
        ptr++;
    }
    return NULL;  // Jeśli nie znaleziono gracza
}
AMCOM_ObjectState* ALG_FindObject(uint8_t object_type){
    AMCOM_ObjectState * ptr = ALG_GameDetails->objects;
    
    // uint16_t number = (uint16_t)ALG_GameDetails->;
    
    for (uint8_t i = 0 ; i < ALG_GameDetails->objects_total; ++i){
        if(ptr->objectNo == object_type){
            return ptr;
        }else{
            ptr++;
        }
    }
    return ptr;
}

AMCOM_ObjectState * ALG_FindNearestObject(uint8_t object_type){
    AMCOM_ObjectState * player = ALG_FindPlayer();
    AMCOM_ObjectState * nearest_object = NULL;
    float min_distance = 10000.0f;
    for (uint8_t i = 0; i< ALG_GameDetails->objects_total; ++i){
        AMCOM_ObjectState * current_object = &ALG_GameDetails->objects[i];
        if (current_object->objectType ==object_type ) {
            float distance = ALG_GetDistance(player,current_object);
            if( distance < min_distance){
                min_distance = distance;
                nearest_object  = current_object;
            }
        }
    }
    return nearest_object;
}


void AMCOM_Print(AMCOM_ObjectState * current_object, uint8_t objects_amount){
   char * types[]  = {"PLAYA","TRANSISTOR", "SPARK", "GLUE"};
        uint8_t type_num;

        for (uint8_t i = 0; i< objects_amount ; ++i){
            
            type_num = current_object[i].objectType;
            
            printf("\nO.number: %u \n",current_object[i].objectNo );
            printf("O.type:   %s \n",types[type_num] );
            printf("O.HP:     %d \n",current_object[i].hp );
            printf("O.POSX:   %f \n",current_object[i].x );
            printf("O.POSY:   %f \n\n",current_object[i].y );
            
        }
            
}
        

// USER END

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

            AMCOM_NewGameRequestPayload * gameData =(AMCOM_NewGameRequestPayload *)packet->payload ;
            if (ALG_GameDetails->objects == 0) ALG_FillGamedata(gameData);


            // send player name
            AMCOM_NewGameResponsePayload newGameResponse;
            sprintf(newGameResponse.helloMessage, PLAYER_MSG);
            toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &newGameResponse, sizeof(newGameResponse), buf);
            break;
        
        case AMCOM_GAME_OVER_REQUEST:
            printf("GAME_OVER.request. Responding with %s.\n", PLAYER_GONE);
            AMCOM_GameOverResponsePayload gameOverResponse;
            sprintf(gameOverResponse.endMessage, PLAYER_GONE);
            toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &gameOverResponse, sizeof(gameOverResponse), buf);
            break;
        

        case AMCOM_OBJECT_UPDATE_REQUEST:
            printf("OBJECTS.request. Reading the data\n");



            uint8_t objects_amount = packet->header.length / (uint8_t)sizeof(AMCOM_ObjectState);
            AMCOM_ObjectState * current_object = (AMCOM_ObjectState *)packet->payload;
            
            if (ALG_GameDetails->objects_registered == 0) ALG_FillObjects(current_object, objects_amount);
            
            // ALG_PrintObjects();
            // AMCOM_Print(current_object, objects_amount);
         
            /*
                TO DO:
                ANALYZE WHAT TO DO:
                    - CHECK WHAT'S CLOSER, EAT A PLAYER(WHO CAN GET LARGER, IF CLOSER THEN NEAREST FOOD)
                    
            */  
            
            break;  

        case AMCOM_MOVE_REQUEST:
            printf("MOVE.request. Responding with MOVING\n");

            AMCOM_MoveResponsePayload moveResponse;
            
            AMCOM_ObjectState * player =  ALG_FindPlayer();
            AMCOM_ObjectState * target = ALG_FindNearestObject(1);
            if(target != NULL){
                moveResponse.angle = ALG_GetRelativeAngle(player, target  );
            }else{
                // moveResponse.angle = 1.4f;
            }

            
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

    ALG_GameDetails = (ALG_GameDetails_t * )malloc(sizeof(ALG_GameDetails_t));


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
    
    free(ALG_GameDetails);
    closesocket(ConnectSocket);
    // Clean up
    WSACleanup();


    return 0;
}


