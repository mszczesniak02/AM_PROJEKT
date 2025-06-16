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

#define AMCOM_OBJECT_TYPES_AMOUNT 4
#define AMCOM_MAX_OBJECT_TYPES_NAME_LENGTH 12




int DUPA = 0;

typedef struct ObjectNode {
    AMCOM_ObjectState data;
    struct ObjectNode* next;
} ObjectNode_t;





typedef struct {
    
    AMCOM_ObjectState objects[AMCOM_MAX_OBJECT_UPDATES];
    
    ObjectNode_t* players;
    ObjectNode_t* static_objects;

    uint8_t objects_max;
    uint8_t objects_total;

    uint8_t objects_registered;
    
    
    uint8_t players_total;
    uint8_t player_num;    
    uint8_t object_follow;

}ALG_GameDetails_t;

// add new object 
ALG_GameDetails_t *  ALG_GameDetails = NULL; // global register of game details, made global for the sole purpose of not messing up with already-established tcp connectors and other pre-made funcionalities 

ObjectNode_t *  head_player     = NULL;
ObjectNode_t *  head_object     = NULL;



// objects are unique, they cannot be duplicates
void ALG_addObject(ObjectNode_t * objectList, AMCOM_ObjectState * object){
    ObjectNode_t * newObj = (ObjectNode_t * )malloc(sizeof(ObjectNode_t));

    newObj->data = *object;
    newObj->next = NULL;

    // check if null / head
    if (objectList == NULL){
        objectList = newObj;
    }else{
        ObjectNode_t * ptr = objectList;
        while(ptr->next != NULL){
        printf("pppp?\n");

            ptr = ptr->next;
        }
        ptr->next = newObj;
    }

}

ObjectNode_t* ALG_findObject(ObjectNode_t * objectList, uint8_t object_type, uint8_t object_num){
    ObjectNode_t* result = NULL;
    ObjectNode_t* ptr = objectList;
    while(ptr != NULL){
        if (ptr->data.objectNo == object_num && ptr->data.objectType == object_type){
            return ptr;
        }else{
            ptr = ptr->next;
        }
    }

    printf("Object not found!\n");
    return result;
}

void ALG_updateObject(ObjectNode_t * objectList, uint8_t object_type, uint8_t object_num, AMCOM_ObjectState * theUpdate){

    ObjectNode_t * object = ALG_findObject(objectList, object_type, object_num);
    if (object == NULL){
        printf("Object not in the list, could not update");
    }else{
        object->data = *theUpdate;
        // memcpy(&(object->data), theUpdate, sizeof(AMCOM_ObjectState));
    }

}



void ALG_GameDetailsInit(){

    ALG_GameDetails->objects_total          = 0;
    ALG_GameDetails->player_num             = 0;
    ALG_GameDetails->players_total          = 0;
    ALG_GameDetails->object_follow          = 0;
    ALG_GameDetails->objects_max = AMCOM_MAX_OBJECT_UPDATES;
    
    ALG_GameDetails->static_objects = head_object;
    ALG_GameDetails->players        = head_player;

    
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


int ALG_CheckList(AMCOM_ObjectState * object){ // linear check for existance in the object register
    AMCOM_ObjectState * objects = ALG_GameDetails->objects;

    for (uint8_t i = 0; i< ALG_GameDetails->objects_max; ++i){
        if (objects->objectNo == object->objectNo && 
        objects->objectType == object->objectType){
            return 1;
        }

    }
    return 0;
}

void ALG_FillGamedata(AMCOM_NewGameRequestPayload * game_data){

    printf("\nplayers_total: %u, player_num:%u.\n", game_data->numberOfPlayers, game_data->playerNumber);

    ALG_GameDetails->players_total = game_data->numberOfPlayers;
    ALG_GameDetails->player_num = game_data->playerNumber;
}

void ALG_FillObjects(AMCOM_ObjectState * object, uint8_t object_amount){
    for(uint8_t i = 0 ; i<object_amount; ++i){
        // check if object exists, if it does, dont put it in
        if(!ALG_CheckList(&object[i])){
            ALG_GameDetails->objects[i] = object[i];
        }
    }
    ALG_GameDetails->objects_total += object_amount;
}

void ALG_FillObjects2(AMCOM_ObjectState * object, uint8_t object_amount){
    for(uint8_t i = 0 ; i<object_amount; ++i){
        // check if object exists, if it does, dont put it in
        if (object->objectType == 0){
            
            ObjectNode_t * ptr;
            if ((ptr = ALG_findObject(ALG_GameDetails->players, object->objectType, object->objectNo)) == NULL){
                ALG_addObject(  ALG_GameDetails->players, object);
            }else{
                ALG_updateObject(ALG_GameDetails->players,object->objectType, object->objectNo, object );
            }
        }else{
            ALG_addObject(ALG_GameDetails->static_objects, object);
        }
    }
    ALG_GameDetails->objects_total += object_amount;
}




void ALG_PrintObject(AMCOM_ObjectState * object){
        
    printf("\nO.number: %u \n",object->objectNo );
    printf("O.type:   (%u) \n" , object->objectType );
    printf("O.HP:     %d \n",object->hp );
    printf("O.POSX:   %f \n",object->x );
    printf("O.POSY:   %f \n\n",object->y );

}
void printA(){
    ObjectNode_t * ptr = ALG_GameDetails->static_objects;
    while(ptr != NULL){
        ALG_PrintObject(&ptr->data);
        ptr = ptr->next;
    }
    ptr = ALG_GameDetails->players;
    while(ptr != NULL){
        ALG_PrintObject(&ptr->data);
        ptr = ptr->next;
    }
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

AMCOM_ObjectState * ALG_FindObject(uint8_t object_type, uint8_t object_number) {
    for (uint8_t i = 0; i < ALG_GameDetails->objects_total; ++i) {
        printf("\nTrying to find object: %u num %u.\n", object_type, object_number);

        if (ALG_GameDetails->objects[i].objectType == object_type && 
            ALG_GameDetails->objects[i].objectNo == object_number) {
            printf("\nObject found: (x,y) = (%f, %f).\n", 
                   ALG_GameDetails->objects[i].x, 
                   ALG_GameDetails->objects[i].y);
            return &ALG_GameDetails->objects[i];
        }
    }
    printf("ALG_FindObject(): Object not found.\n");
    return NULL;  // If object not found
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
            
            AMCOM_IdentifyResponsePayload identifyResponse;
            printf("IDENTIFY.request. Responding with %s.\n", PLAYER_NAME);
            
            sprintf(identifyResponse.playerName, PLAYER_NAME);
            toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
            break;
    
        case AMCOM_NEW_GAME_REQUEST:
            printf("NEW_GAME.request. Responding with %s.\n", PLAYER_MSG);

            // GET THE DATA FROM A PACKET AND INSERT THE IN TO REGISTER
            AMCOM_NewGameRequestPayload * gameData =(AMCOM_NewGameRequestPayload *)packet->payload ;
            ALG_FillGamedata(gameData);


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
            AMCOM_ObjectState * objects = (AMCOM_ObjectState *)packet->payload;
            
            ALG_FillObjects(objects, objects_amount);
            ALG_FillObjects2(objects, objects_amount);
            // printA();
            /*
                TO DO:
                ANALYZE WHAT TO DO:
                    - CHECK WHAT'S CLOSER, EAT A PLAYER(WHO CAN GET LARGER, IF CLOSER THEN NEAREST FOOD)
                    
            */  
            
            break;  

        case AMCOM_MOVE_REQUEST:
            printf("MOVE.request. Responding with MOVING\n");


            AMCOM_MoveResponsePayload moveResponse;
            // ALG_PrintObjects();
            

            // AMCOM_ObjectState * player =  ALG_FindObject(0, ALG_GameDetails->player_num);   // 0 - player
            // if(player != NULL){
                    
                
            //     if ( DUPA > 2){
            //         AMCOM_ObjectState * object =  ALG_FindObject(1, 0);            // 1 - transistor 
            //         float new_angle = ALG_GetRelativeAngle(player,object );
            //         printf("newAngle %f\n", new_angle);
            //     }else{
            //         DUPA++;
            //     }
            //     // try to calculate angle
                moveResponse.angle = 1;
    
                toSend = AMCOM_Serialize(AMCOM_MOVE_RESPONSE, &moveResponse, sizeof(moveResponse), buf);
            }
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
    
    ALG_GameDetailsInit();

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
    free(head_object);
    free(head_player);
    free(ALG_GameDetails);

    closesocket(ConnectSocket);
    // Clean up
    WSACleanup();


    return 0;
}


