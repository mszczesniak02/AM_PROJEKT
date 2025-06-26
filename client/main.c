#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
// #include <stdlib.h>

#include "amcom.h"
#include "amcom_packets.h"
#include <math.h>

#define PLAYER_NAME "White power."
#define PLAYER_MSG "bitches come and go."
#define PLAYER_GONE "bullshit."
#define EPSILON 10.0f
#define EPSILON_ANGLE 0.1745 // 10 deg, 10 * 180 / PI = 0.1745


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
        printf("-------obj start----------\n");
        printf("O.type:   %u \n",o->objectType );
        printf("O.number: %lu \n",o->objectNo );
        printf("O.HP:     %d \n",o->hp );
        printf("O.POSX:   %f \n",o->x );
        printf("O.POSY:   %f \n",o->y );
        printf("-------obj end----------\n");

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
    
    assert(node);

    node->data = (AMCOM_ObjectState*)malloc(sizeof(AMCOM_ObjectState));
    
    assert(node->data);

    memcpy(node->data, o, sizeof(AMCOM_ObjectState));
    
    
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
            if ( current->data != NULL) free(current->data); // to psuje mi z jakiegos powodu 
        
        free(current);
        current=next;
    }    
    *head = NULL;
}

ll_t* head_p = NULL;
ll_t* head_t = NULL;
ll_t* head_s = NULL;
ll_t* head_g = NULL;


typedef struct next_stop{
    float x;
    float y;
    float dist;
    float angle;
} Next_stop_t;

typedef struct GameDetails{
    uint8_t total_players;
    uint8_t my_player;
    uint8_t current_angle;
    Next_stop_t next_stop;

    uint8_t next;

}GameDetails_t;


GameDetails_t * GameDetails = NULL;

void GameDetailsInit(void){
    GameDetails->current_angle = 0;
    GameDetails->my_player = 0;
    GameDetails->total_players = 0;
    GameDetails->next_stop.angle = 0.0f;
    GameDetails->next_stop.dist = 0.0f;
    GameDetails->next_stop.x = 0.0f;
    GameDetails->next_stop.y = 0.0f;
    GameDetails->next = 0;

}





/*
    1 - eat the nearest food in vacinity, defined by close distance to each other(maybe like 50/100 ), then go after the player with the least health to eat him if I am bigger by a factor of 2/3/4...

    Analyze the vacinities once, 

    przejdz przez wszystkie obiekty do zjedzenia, pakiety powinny posiadac wielkości dwie więcej, angle_to_nearest food, distance to nearest food
    order by the closest distances and distace to the player
    -> somehow get the order of which to go after, example -> 1 - 8- 3 -5 -6, be able to navigate them knowing that theyre ordered from 12 to 0. (packet size and ye).

    -> player distance fast calc to get their position and to EAT THEM MFs.



    funcs: calc distace, calc angle, sort by distance, 



*/

AMCOM_ObjectState * findObject( uint8_t type, uint16_t num){
    ll_t* p = NULL;

    switch(type){
        case 0: p = head_p; break;
        case 1: p = head_t; break;
        case 2: p = head_s; break;
        case 3: p = head_g; break;
    }

    while(p != NULL){
        
        if( p->data->objectType == type &&  p->data->objectNo == num){
            return p->data;
        }
        p = p->next;
    }
    return NULL;
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
        // PrintObject(&state[i]);
        // printNode(&p);   
    }
}

void ALG_Transpose(AMCOM_ObjectState * object, float * x, float * y){ //transpose from -500/500 to 0/1000
    *x = object->x + 500.0f;
    *y = object->y + 500.0f;
}

float ALG_GetDistance(AMCOM_ObjectState * object_origin,AMCOM_ObjectState * object_dest ){
    float xo = 0.0f, yo = 0.0f;
    float xd = 0.0f, yd = 0.0f;

    ALG_Transpose(object_origin, &xo, &yo);
    ALG_Transpose(object_dest, &xd, &yd);

    // calculate distance in straight line from origin to dest, 
    return sqrt(  
                (xd - xo) * (xd - xo) + // (x_dest-x_origin)^2
                (yd - yo) * (yd - yo)   
                );
}

float ALG_GetAbsoluteAngle(AMCOM_ObjectState * object_origin, AMCOM_ObjectState * object_dest){

    float distance_x = object_dest->x - object_origin->x;
    float distance_y = object_dest->y - object_origin->y;

    float angle = atan2f(distance_y, distance_x);
    return angle;
}



float ALG_Move(AMCOM_ObjectState * player, AMCOM_ObjectState * food){
    float epsilon = 5.0f;

    if(player && food){
        float angle     = ALG_GetAbsoluteAngle( player,food );
        float distance  = ALG_GetDistance( player, food );
        printf("Current (d,phi) to food: (%f, %f)\n", distance, angle );
        return angle;
    }
    return -1.0f;
}

uint8_t ALG_SparkInWay(AMCOM_ObjectState * player, float food_angle, float food_distance){
    ll_t * sparks = head_s;

    while(sparks != NULL){
        
        float obsticle_angle = ALG_GetAbsoluteAngle(player, sparks->data);
        float obsticle_distance = ALG_GetDistance(player, sparks->data);

        if ( fabsf(food_angle -  obsticle_angle  ) < EPSILON_ANGLE ){
            if (obsticle_distance < food_distance)  return 1; // will hit the obsticle
        }// angle bigger then epsilon
        sparks = sparks->next;
    }
    return 0;
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

            break;  

        case AMCOM_MOVE_REQUEST:
            printf("MOVE.request. Responding with MOVING\n");

            enum {
                PLAYER,
                FOOD,
                SPARK,
                GLUE
            };

          
            
            AMCOM_ObjectState * player = findObject(PLAYER, GameDetails->my_player);
            assert(player);

            AMCOM_ObjectState * food = NULL;      

            while(1){
                
                food = findObject(FOOD, GameDetails->next );
                assert(food);

                if( ALG_GetDistance(player,food) <= EPSILON){ // assumption that food is eaten
                    GameDetails->next++;
                    continue;
                }
                
                float angle_response =  ALG_GetAbsoluteAngle(player,food),
                distance_responce = ALG_GetDistance(player, food);

                if(ALG_SparkInWay(player, angle_response, distance_responce) ){ // if spark in the way, go for another food
                    GameDetails->next++;
                    food = NULL;
                    continue;
                }

                break;
            }


            AMCOM_MoveResponsePayload moveResponse;
      

            float received_angle = ALG_Move(player, food);
      
            assert(received_angle != -1.0f);
            moveResponse.angle = received_angle;
            

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


