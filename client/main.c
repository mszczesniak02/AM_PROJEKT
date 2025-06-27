#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


#include "amcom.h"
#include "amcom_packets.h"
#include <math.h>

#define PLAYER_NAME "White power."
#define PLAYER_MSG "bitches come and go."
#define PLAYER_GONE "bullshit."
#define EPSILON 10.0f
#define EPSILON_DISTANCE 75.0f
#define EPSILON_ANGLE 0.5f // 10 deg, 10 * 180 / PI = 0.1745
#define CLUSTER_RADIUS 100.0f

#define FOOD_TO_EAT_COEF 0.35


#define REQ_RESP_LOG 0
#define ALGORITH_DETAILS_LOG 1

float force_move = 12.0;



void PrintObject(AMCOM_ObjectState * o){
    if(o){
        printf("-------obj start----------\n");
        printf("O.type:   %u \n",o->objectType );
        printf("O.num`r: %lu \n",o->objectNo );
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



ll_t* head_p = NULL;
ll_t* head_t = NULL;
ll_t* head_s = NULL;
ll_t* head_g = NULL; // ignore the glue, not worth the trouble


typedef struct next_stop{
    float dist;
    float angle;
    float x;
    float y;
} next_stop_t;

typedef struct GameDetails{
    uint8_t total_players;
    uint8_t my_player;
    uint8_t current_angle;
    
    next_stop_t* next_stop;
    uint8_t * next_stop_visited;
    uint8_t next_stop_count;

    uint8_t players_c;
    uint8_t transistor_c;
    uint8_t sparks_c;
    uint8_t glue_c;

    uint8_t next ;

}GameDetails_t;


GameDetails_t * GameDetails = NULL;

void GameDetailsInit(void){
    GameDetails->current_angle = 0;
    GameDetails->my_player = 0;
    GameDetails->total_players = 0;
   
    // maybe init the next_top lists
    GameDetails->next_stop_count = 0;
    GameDetails->next_stop = NULL;
    GameDetails->next_stop_visited = NULL;

    GameDetails->next = 0;

    GameDetails->players_c = 0;
    GameDetails->transistor_c = 0;
    GameDetails->sparks_c = 0;
    GameDetails->glue_c = 0;

}

void NextStop_Init(void){

    GameDetails->next_stop = (next_stop_t*)malloc(sizeof(next_stop_t) * GameDetails->next_stop_count);
    assert(GameDetails->next_stop);
    GameDetails->next_stop_visited = (uint8_t*)calloc(GameDetails->next_stop_count,sizeof(uint8_t));
    assert(GameDetails->next_stop_visited);

}

void NextStop_Free(void){
    free(GameDetails->next_stop_visited);
    free(GameDetails->next_stop);
}

void clearList(ll_t ** head){
    ll_t* current =  *head;
    ll_t* next;
    while(current != NULL){
        next = current->next;
            if ( current->data != NULL) free(current->data); 
        free(current);
        current=next;
    }    
    *head = NULL;
    
    GameDetails->players_c = 0;
    GameDetails->transistor_c = 0;
    GameDetails->sparks_c = 0;
    GameDetails->glue_c = 0;
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
            case 0: p = head_p; pushNode(&head_p, &state[i]);   GameDetails->players_c++         ;break;
            case 1: p = head_t; pushNode(&head_t, &state[i]);   GameDetails->transistor_c++      ;break;
            case 2: p = head_s; pushNode(&head_s, &state[i]);   GameDetails->sparks_c++          ;break;
            case 3: p = head_g; pushNode(&head_g, &state[i]);   GameDetails->glue_c++            ;break;
        }
    }
}

/*
    Get (x,y) coordinates from (-500,500) to (0, 1000)
    Doesn't change the object itself, puts the transposed data into func parameters 'x' and 'y'
*/
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


typedef struct RelativePosition{
    AMCOM_ObjectState * data;
    float distance_to_origin;
    float angle_to_origin;

} RelativePosition_t;

RelativePosition_t* RelativePosition_Init(AMCOM_ObjectState* o){
    RelativePosition_t * node = (RelativePosition_t*)malloc(sizeof(RelativePosition_t));
    
    assert(node);
    node->distance_to_origin = -1.0f;
    node->angle_to_origin = -1.0f;

    node->data = o;
    return node;
}

void RelativePosition_free(RelativePosition_t* o){
    // clearList(o->data);
    o->data = NULL; // dont clear the list yet

    free(o);
}

void RelativePosition_Print(RelativePosition_t* o){
    printf("(%u, %u) - ", o->data->objectType,o->data->objectNo);
    printf("(%f,%f)\n", o->distance_to_origin, o->angle_to_origin);
}

uint8_t CountNodes(ll_t** head){
    uint8_t count = 0;

    assert(&head);

    ll_t * current = *head;
    while(current != NULL){
        count++;
        current = current->next;
    }
    assert(count != 0);
    return count;

}

int ClusterCompair(const void* a, const void* b){
    RelativePosition_t* obj_a = *( (RelativePosition_t**) a);
    RelativePosition_t* obj_b = *( (RelativePosition_t**) b);

    if (obj_a->distance_to_origin < obj_b->distance_to_origin) {
        return -1; // obj_a jest bliżej, więc jest "mniejszy"
    }
    if (obj_a->distance_to_origin > obj_b->distance_to_origin) {
        return 1; // obj_a jest dalej, więc jest "większy"
    }

    return 0;
}


void sortClustersPosisions(RelativePosition_t * node[], uint8_t count){
    qsort(node, count, sizeof(RelativePosition_t*), ClusterCompair);
}



void printClusterNode(RelativePosition_t* node[], uint8_t count){
    printf("(type, num) - (d,phi)\n");
    
    for(uint8_t i = 0 ; i< count; ++i){
        RelativePosition_Print(node[i]);
    }
}

typedef struct pos_xy_t{
    float dist;
    float angle;
    float x;
    float y;
}pos_xy_t;



typedef struct node2{
    pos_xy_t * data;
    struct node2 * next;
}cluster_element_t;


cluster_element_t* createClusterElement(pos_xy_t * data){
    cluster_element_t * node = (cluster_element_t*)malloc(sizeof(cluster_element_t));
    assert(node);

    node->data  = (pos_xy_t*)malloc(sizeof(pos_xy_t));    
    assert(node->data);

    memcpy(node->data, data, sizeof(pos_xy_t));
    node->next = NULL;
    return node;
}

typedef struct node1 {
    cluster_element_t * data;
    struct node1 * next;
}cluster_list_t ;

void pushClusterElement(cluster_element_t** head, pos_xy_t * o ){
    
    cluster_element_t* node = createClusterElement(o);
    node->next = *head;
    *head = node;
}


cluster_list_t * createCluster(cluster_element_t * o){
    
    cluster_list_t* node = (cluster_list_t* )malloc( sizeof(cluster_list_t) );
    assert(node);

    node->data = (cluster_element_t*)malloc(sizeof(cluster_element_t*));
    
    assert(node->data);
    memcpy(node->data, o, sizeof(cluster_element_t));
    
    node->next = NULL;
    return node;
}

void pushCluster(cluster_list_t** head, cluster_element_t * o ){
    
    cluster_list_t* node = createCluster(o);
    node->next = *head;
    *head = node;
    
}

void printCluster(cluster_list_t** head){
    cluster_list_t* current = *head;

    while(current != NULL){

        printf("[");
        
        cluster_element_t  * element = current->data;
        while(element != NULL){
            printf(" [%f, %f], ",element->data->dist, element->data->angle  );
            element = element->next;
        }
        printf("] \n");
        current = current->next;
    }
}




void reverseElementsList(cluster_element_t** head_ref) {
    cluster_element_t* head = *head_ref;
    // Jeśli lista jest pusta lub ma tylko jeden element, nie rób nic
    
    assert(head);

    cluster_element_t* prev = NULL;
    cluster_element_t* current = head;
    cluster_element_t* next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    *head_ref = prev;
}

// Główna funkcja, która odwraca listę klastrów oraz elementy wewnątrz każdego z nich
void reverseClusterList(cluster_list_t** list_head_ref) {
    assert(list_head_ref);

    // 1. Odwróć listę elementów w każdym klastrze
    cluster_list_t* current_cluster = *list_head_ref;
    while (current_cluster != NULL) {
        reverseElementsList(&(current_cluster->data));
        current_cluster = current_cluster->next;
    }

    // 2. Odwróć główną listę klastrów
    cluster_list_t* head = *list_head_ref;
    // Jeśli lista jest pusta lub ma tylko jeden element, nie rób nic
    assert(head);

    cluster_list_t* prev = NULL;
    cluster_list_t* current = head;
    cluster_list_t* next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    *list_head_ref = prev;
}


void clearClusterElements(cluster_element_t** head){
    cluster_element_t* current =  *head;
    cluster_element_t* next;
    while(current != NULL){
        next = current->next;
        free(current->data); // WAŻNE: zwolnij wewnętrzny wskaźnik 'data'
        free(current);       // Dopiero teraz zwolnij węzeł listy
        current=next;
    }    
    *head = NULL;
}

void clearClusterList(cluster_list_t** head){
    cluster_list_t* current =  *head;
    cluster_list_t* next;
    while(current != NULL){
        next = current->next;
        clearClusterElements(&(current->data)); // Free the inner list of elements
        free(current);       // Free the cluster node itself
        current=next;
    }
    *head = NULL;
}

void getClusters(RelativePosition_t* nodes[], uint8_t count);


void calculateClustersPosition(ll_t** head, AMCOM_ObjectState * player, uint8_t count ){
    assert(&head);
    
    ll_t * current = *head;
   
    RelativePosition_t* node[count];

    for(uint8_t i = 0; i< count; ++i){
        
        node[i] = RelativePosition_Init(current->data);
        node[i]->distance_to_origin = ALG_GetDistance(player, node[i]->data); // set the distance from the first element in linked list 'head' 
        node[i]->angle_to_origin = ALG_GetAbsoluteAngle(player, node[i]->data);
        current = current->next;
        // RelativePosition_Print(node[i]);
    }

    printf("Before");
    printClusterNode(node, count);
    sortClustersPosisions(node, count);
    printf("After");
    printClusterNode(node, count);

    getClusters(node, count);
    
    


    for(uint8_t i = 0; i< count; ++i) RelativePosition_free(node[i]);
    // sort the data 


};


void getClusters(RelativePosition_t* nodes[], uint8_t count) {
    cluster_list_t* list = NULL;
    uint8_t* visited = (uint8_t*)calloc(count, sizeof(uint8_t));

    for (uint8_t i = 0; i < count; ++i) {
        if (!visited[i]) {
            cluster_element_t* current_cluster_elements = NULL;

            // 1. Dodaj element 'i' jako pierwszy do nowego klastra
            pos_xy_t p_i = {
                nodes[i]->distance_to_origin, 
                nodes[i]->angle_to_origin,
                nodes[i]->data->x,
                nodes[i]->data->y
            };
            pushClusterElement(&current_cluster_elements, &p_i);
            visited[i] = 1;

            // 2. Szukaj kolejnych elementów do tego klastra
            for (uint8_t j = i + 1; j < count; ++j) {
                if (!visited[j]) {
                    float distance_diff = fabsf(nodes[i]->distance_to_origin - nodes[j]->distance_to_origin);
                    float angle_diff = fabsf(nodes[i]->angle_to_origin - nodes[j]->angle_to_origin);

                    if (distance_diff < EPSILON_DISTANCE && angle_diff < EPSILON_ANGLE) {
                        // 3. Jeśli 'j' jest blisko, dodaj 'j' do klastra
                        pos_xy_t p_j = {
                            nodes[j]->distance_to_origin, 
                            nodes[j]->angle_to_origin,
                            nodes[j]->data->x,
                            nodes[j]->data->y,

                        };
                        pushClusterElement(&current_cluster_elements, &p_j);
                        visited[j] = 1;
                    }
                }
            }

            // 4. Po pętli 'j', cały klaster jest gotowy. Dodaj go do listy.
            // NIE zwalniaj tutaj `current_cluster_elements`!
            pushCluster(&list, current_cluster_elements);
        }
    }

    printf("Food Clusters found\n");
    if(ALGORITH_DETAILS_LOG) printCluster(&list);
    
    
    reverseClusterList(&list);
    printf("Food Clusters sorted\n");
    if(ALGORITH_DETAILS_LOG)  printCluster(&list);

    // insert the calculated 

    uint8_t amount_to_eat = 0;
    if (GameDetails->total_players > 0) {
        amount_to_eat = (GameDetails->transistor_c / GameDetails->total_players) * FOOD_TO_EAT_COEF;
    }
    // Make sure we try to eat at least one cluster if transistors are available
    if (amount_to_eat == 0 && GameDetails->transistor_c > 0) {
        amount_to_eat = 1;
    }

    GameDetails->next_stop_count = amount_to_eat; 
    NextStop_Init();
    
  
    

    // Pamiętaj, aby na końcu zwolnić całą listę klastrów, gdy nie będzie już potrzebna.
    clearClusterList(&list);
    free(visited);
}



void amPacketHandler(const AMCOM_Packet* packet, void* userContext) {
    uint8_t buf[AMCOM_MAX_PACKET_SIZE];              // buffer used to serialize outgoing packets
    size_t toSend = 0;                               // size of the outgoing packet
    SOCKET ConnectSocket  = *((SOCKET*)userContext); // socket used for communication with the server

    switch (packet->header.type) {

        case AMCOM_IDENTIFY_REQUEST:

            if (REQ_RESP_LOG) printf("IDENTIFY.request. Responding with %s.\n", PLAYER_NAME);
            

            AMCOM_IdentifyResponsePayload identifyResponse;
            sprintf(identifyResponse.playerName, PLAYER_NAME);
            toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
            break;
    
        case AMCOM_NEW_GAME_REQUEST:
            
            if (REQ_RESP_LOG) printf("NEW_GAME.request. Responding with %s.\n", PLAYER_MSG);        
            
            AMCOM_NewGameResponsePayload newGameResponse;
            sprintf(newGameResponse.helloMessage, PLAYER_MSG);

          
            AMCOM_NewGameRequestPayload * ptr = (AMCOM_NewGameRequestPayload  *)packet->payload;
            
            GameDetails->my_player = ptr->playerNumber;
            GameDetails->total_players = ptr->numberOfPlayers;


            toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &newGameResponse, sizeof(newGameResponse), buf);
            break;
        
        case AMCOM_GAME_OVER_REQUEST:
            if (REQ_RESP_LOG) printf("GAME_OVER.request. Responding with %s.\n", PLAYER_GONE);
            AMCOM_GameOverResponsePayload gameOverResponse;
            sprintf(gameOverResponse.endMessage, PLAYER_GONE);
            
            
            clearList(&head_p);
            clearList(&head_t);
            clearList(&head_s);
            clearList(&head_g);

            
            
            toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &gameOverResponse, sizeof(gameOverResponse), buf);
            break;
        

        case AMCOM_OBJECT_UPDATE_REQUEST:
            if (REQ_RESP_LOG) printf("OBJECTS.request. Reading the data\n");

            clearList(&head_p); // players are allways sent


            uint8_t objects_amount = packet->header.length / (uint8_t)sizeof(AMCOM_ObjectState);//  packet size-> 12
            AMCOM_ObjectState * current_object = (AMCOM_ObjectState *)packet->payload;
            
            fetchObjects(current_object, objects_amount);

            break;  

        case AMCOM_MOVE_REQUEST:
            if (REQ_RESP_LOG) printf("MOVE.request. Responding with MOVING\n");
            
            enum {
                PLAYER,
                FOOD,
                SPARK,
                GLUE
            };

            AMCOM_MoveRequestPayload * gameRound = (AMCOM_MoveRequestPayload *)packet->payload; 
            printf("Round: %llu \n", gameRound->gameTime);

            AMCOM_ObjectState * player = findObject(PLAYER, GameDetails->my_player);
            AMCOM_ObjectState * food_origin = findObject(FOOD,0);
            
            assert(player);

            

            if ( gameRound->gameTime == 0 ){
                
                calculateClustersPosition(&head_t, player, CountNodes(&head_t) );
                
            }

            AMCOM_ObjectState * food = NULL;      

            while(1){
                
                food = findObject(FOOD, GameDetails->next );
                assert(food); // when next is bigger then the amount of possible food to eat, assertion fails due to skipping the valid food.

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



    if (REQ_RESP_LOG) {
        printf("\n----- REQ,RESP LOGs ENABLED-----\n\n");
    }else{
        printf("\n----- REQ,RESP LOGs DISABLED-----\n\n");
    }

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


