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



#define EPSILON_DISTANCE 40.0f
#define EPSILON_ANGLE 0.4f // 10 deg, 10 * 180 / PI = 0.1745
#define CLUSTER_RADIUS 100.0f

#define FOOD_TO_EAT_COEF 0.4
#define PI 3.141592
#define BYPASS_ARG 255 // used once, value too big .. coś tam
#define REQ_RESP_LOG 0

#define ALGORITHM_DATA_LOG 0



/**
 * \brief Prints details of an AMCOM_ObjectState.
 *  
 * @param o Structure describing the state of a single game object 
 *               
 */
void PrintObject(AMCOM_ObjectState * o);


/**
 * \brief Stack like node, holds AMCOM_ObjectState data
 *
 */
typedef struct node{
    AMCOM_ObjectState * data;
    struct node * next;
}ll_t;


/**
 * \brief Create an instance of a new Node of type ll_t.
 *  Copies the AMCOM_ObjectState into itself.
 * 
 * @param o Structure describing the state of a single game object 
 *               
 */
ll_t * createNode(AMCOM_ObjectState * o);

/**
 * \brief Push node into the head element
 *  
 * @param head the first element of a stack
 * @param o Structure describing the state of a single game object 
 *               
 */
void pushNode(ll_t** head, AMCOM_ObjectState * o );

ll_t* head_p = NULL;
ll_t* head_t = NULL;
ll_t* head_s = NULL;


/**
 * \brief Structure that holds sorted data for later usage in main game loop.
 * 
 */
typedef struct next_stop{
    float dist;
    float angle;
    AMCOM_ObjectState * data;
} next_stop_t;

/**
 * \brief Structure that holds data avaiable globaly, used in order to not change the TCP connection code.
 *  @param current_stop         holds the current element of a loop
 *  @param next_stop            holds the sorted elements 'to eat' 
 *  @param next_stop_visited    flags for already-eaten transistors 
 *  @param next_stop_count      stores the amount of next_stop elements         
 *  @param players_c            count of player entities, same goes for the rest 
 */
typedef struct GameDetails{
    uint8_t my_player;
    float current_angle;
        
    uint8_t current_stop;
    next_stop_t* next_stop;
    uint8_t * next_stop_visited;
    uint8_t next_stop_count;

    uint8_t players_c;
    uint8_t transistor_c;
    uint8_t sparks_c;

}GameDetails_t;


/**
 * \brief globaly avaiable structure 
 */
GameDetails_t * GameDetails = NULL;


/**
 * \brief initializer of the GameDetails structure.
 */
void GameDetails_init(void);

/**
 * \brief initializer of the NextStop_t structure. Holds the desired foods list.
 */
void NextStop_Init(void);

/**
 * \brief free memory of initialized next_stop structure.
 */
void NextStop_Free(void);

/**
 * \brief Erase the whole stack and its data, free the memory. Mainly used to clear data after finished game
 * \param head stack head to erase
 */
void clearNode(ll_t ** head);


/**
 * \brief Finds the object described by the parameters. Decides on the head based on the type of an object.
 * \param type AMCOM_ObjectState type (0-player, 1-transistor,2-spark,3-glue)
 * \param num AMCOM_ObjectState number of an element
 * 
 */
AMCOM_ObjectState * findObject( uint8_t type, uint16_t num);


/**
 * \brief Finds the object described by the parameters. Decides on the head based on the type of an object.
 * \param state AMCOM_ObjectState array with data to fetch
 * \param count amount of AMCOM_ObjectState packets to fetch
 * 
 */
void fetchObjects(AMCOM_ObjectState * state, uint8_t count);

/**
 * \brief  Get (x,y) coordinates from (-500,500) to (0, 1000)
    Doesn't change the object itself, puts the transposed data into func parameters 'x' and 'y'
 * \param object AMCOM_ObjectState object
 * \param x return value 
 * \param y return value  

 */
void Transpose(AMCOM_ObjectState * object, float * x, float * y);



float GetDistance(AMCOM_ObjectState * object_origin,AMCOM_ObjectState * object_dest );

float GetAbsoluteAngle(AMCOM_ObjectState * object_origin, AMCOM_ObjectState * object_dest);


float Move(AMCOM_ObjectState * player, AMCOM_ObjectState * food);

uint8_t SparkInWay(AMCOM_ObjectState * player, AMCOM_ObjectState * food);

/**
 * \brief  Temporary structure used for sorting the NextElement structure.
 */
typedef struct RelativePosition{
    AMCOM_ObjectState * data;
    float distance_to_origin;
    float angle_to_origin;

} RelativePosition_t;


/**
 * \brief  Initializes the structure RelativePostion_t.
 * \param state AMCOM_ObjectState object to initiate with
 */
RelativePosition_t* RelativePosition_Init(AMCOM_ObjectState* o);

/**
 * \brief  Initializes the structure RelativePostion_t.
 * \param o RelativePosition element
 */
void RelativePosition_free(RelativePosition_t* o);


/**
 * \brief  Prints single instance of RelativePosition_t.
 * \param o RelativePosition element
 */
void RelativePosition_Print(RelativePosition_t* o);

/**
 * \brief  Counts and returns the amount of objects in a stack structure.
 * \param head stack head, first element
 */
uint8_t CountNodes(ll_t** head);

/**
 * \brief  Function used as qsort comparator.
 */
int ClusterCompair(const void* a, const void* b);

/**
 * \brief  Quick sort the lists of RelativePostion_t node.
 */
void sortClustersPosisions(RelativePosition_t * node[], uint8_t count);


/**
 * \brief  Print the whole RelativePostion list.
 */
void printClusterNode(RelativePosition_t* node[], uint8_t count);


/**
 * \brief  Stack structure with next_stop_t as data. Used for inner Cluster element
 */
typedef struct node2{
    next_stop_t * data;
    struct node2 * next;
}cluster_element_t;

/**
 * \brief  Create an instance of a stack for cluster_element_t.
 */
cluster_element_t* createClusterElement(next_stop_t * data);

/**
 * \brief  Stack structure with cluster_element_t as data. Outer list, that holds innner list
 */
typedef struct node1 {
    cluster_element_t * data;
    struct node1 * next;
}cluster_list_t ;

/**
 * \brief  push Stack structure into the head, having data as next_stop_t.
 * \param head head element, first alement
 * \param o object to initiate as data
 */
void pushClusterElement(cluster_element_t** head, next_stop_t * o );


/**
 * \brief  Stack structure with cluster_element_t as data. Outer list, that holds innner list, creates new instance
 */
cluster_list_t * createCluster(cluster_element_t * o);


/**
 * \brief  Push onto stack the cluster structure
 */
void pushCluster(cluster_list_t** head, cluster_element_t * o );


/**
 * \brief  Prints the whole cluster
 */
void printCluster(cluster_list_t** head);

/**
 * \brief  Reverse stack order (reverse Cluster helper)
 */
void reverseElementsList(cluster_element_t** head_ref);

/**
 * \brief  Reverse stack order of a cluster_list
 */
void reverseClusterList(cluster_list_t** list_head_ref);

/**
 * \brief clear elements of a clusterElements (stack)
 */
void clearClusterElements(cluster_element_t** head);

/**
 * \brief clear elements of a ClusterList (stack with ClusterElements as data)
 */
void clearClusterList(cluster_list_t** head);

/**
 * \brief  Generate clusters of data and put int int the node. Sets the amout of food to eat
 * \param nodes list of elements used for later processing
 * \param count number of elements
 */
void getClusters(RelativePosition_t* nodes[], uint8_t count);

/**
 * \brief  Calculates the distances and sorts the entities to eat
 */
void calculateClustersPosition(ll_t** head, AMCOM_ObjectState * player, uint8_t count );

/**
 * \brief  Adds the food clusters into the GameDetails global var.
 */
void NextStop_addClusters(cluster_list_t** head);


/**
 * \brief  Determines whether a player has reached the food based on (x,y) location.
 */
uint8_t foodReached(AMCOM_ObjectState * player);



/**
 * \brief  check if element is in  a list
 * \param element element to look for
 * \param arr list to search
 * \param count num of elements in list
 */
uint8_t inList(uint8_t element, const uint8_t arr[], uint8_t count);

/**
 * \brief  find next food avaiable to eat and return a flag for later processing
 * \param excluded list with elements that aren't look at during searching ( list of food blocked by a spark in a straight line)
 * \param count num of elements in count
 * \param player my_player of AMCOM_ObjectState structure 
 */
uint8_t foodNext(const uint8_t excluded[], uint8_t count, AMCOM_ObjectState * player);


/**
 * \brief  Handles the TCP connection and main game loop.
 * \param packet TCP packet to handle
 * \param userContext packet data holder
 */
void amPacketHandler(const AMCOM_Packet* packet, void* userContext);


#define GAME_SERVER "localhost"
#define GAME_SERVER_PORT "2001"

int main(int argc, char **argv) {


    GameDetails = (GameDetails_t*)malloc(sizeof(GameDetails_t));
    GameDetails_init();

    srand(56473892);


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
    free(GameDetails);
    // Clean up
    WSACleanup();

    return 0;
}


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
void GameDetails_init(void){
    GameDetails->current_angle = 0.0f;
    GameDetails->my_player = 0;
    GameDetails->current_stop  = 0;
   
    GameDetails->next_stop_count = 0;
    GameDetails->next_stop = NULL;
    GameDetails->next_stop_visited = NULL;
    

    GameDetails->players_c = 0;
    GameDetails->transistor_c = 0;
    GameDetails->sparks_c = 0;
    

}
void NextStop_Init(void){

    GameDetails->next_stop = (next_stop_t*)malloc(sizeof(next_stop_t) * GameDetails->next_stop_count); // allocate new memory 
    assert(GameDetails->next_stop);
    
    GameDetails->next_stop_visited = (uint8_t*)calloc(GameDetails->next_stop_count,sizeof(uint8_t));
    assert(GameDetails->next_stop_visited);

    GameDetails->next_stop->data = NULL;
}
void NextStop_Free(void){
    free(GameDetails->next_stop_visited);
    free(GameDetails->next_stop);
}

void clearNode(ll_t ** head){
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
   
}
AMCOM_ObjectState * findObject( uint8_t type, uint16_t num){
    ll_t* p = NULL;

    switch(type){
        case 0: p = head_p; break;
        case 1: p = head_t; break;
        case 2: p = head_s; break;
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
       
        switch(state[i].objectType){
            case 0:  pushNode(&head_p, &state[i]);   GameDetails->players_c++         ;break;
            case 1:  pushNode(&head_t, &state[i]);   GameDetails->transistor_c++      ;break;
            case 2:  pushNode(&head_s, &state[i]);   GameDetails->sparks_c++          ;break;
            // case 3:  pushNode(&head_g, &state[i]);   GameDetails->glue_c++            ;break; // I couldn't care less about the glue
        }
    }
}

void Transpose(AMCOM_ObjectState * object, float * x, float * y){ //transpose from -500/500 to 0/1000
    *x = object->x + 500.0f;
    *y = object->y + 500.0f;
}
float GetDistance(AMCOM_ObjectState * object_origin,AMCOM_ObjectState * object_dest ){
    float xo = 0.0f, yo = 0.0f;
    float xd = 0.0f, yd = 0.0f;

    Transpose(object_origin, &xo, &yo);
    Transpose(object_dest, &xd, &yd);

    // calculate distance in straight line from origin to dest, 
    return sqrt(  
                (xd - xo) * (xd - xo) + // (x_dest-x_origin)^2
                (yd - yo) * (yd - yo)   
                );
}

float GetAbsoluteAngle(AMCOM_ObjectState * object_origin, AMCOM_ObjectState * object_dest){

    float distance_x = object_dest->x - object_origin->x;
    float distance_y = object_dest->y - object_origin->y;

    float angle = atan2f(distance_y, distance_x);
    return angle;
}

float Move(AMCOM_ObjectState * player, AMCOM_ObjectState * food){
    if(player && food){
        float angle     = GetAbsoluteAngle( player,food );
        float distance  = GetDistance( player, food );
        printf("Current (d,phi) to food: (%f, %f)\n", distance, angle );
        return angle;
    }
    return -1.0f;
}


uint8_t SparkInWay(AMCOM_ObjectState * player, AMCOM_ObjectState * food){
    ll_t * sparks = head_s;

    while(sparks != NULL){
        
        float obsticle_angle = GetAbsoluteAngle(player, sparks->data);
        float obsticle_distance = GetDistance(player, sparks->data);

        float food_angle = GetAbsoluteAngle(player,food);
        float food_distance = GetDistance(player, food);

        if ( fabsf(food_angle -  obsticle_angle  ) < EPSILON_ANGLE ){
            if (obsticle_distance < food_distance)  return 1; // will hit the obsticle
        }// angle bigger then epsilon
        sparks = sparks->next;
    }
    return 0;
}


RelativePosition_t* RelativePosition_Init(AMCOM_ObjectState* o){
    RelativePosition_t * node = (RelativePosition_t*)malloc(sizeof(RelativePosition_t));
    
    assert(node);
    node->distance_to_origin = -1.0f;
    node->angle_to_origin = -1.0f;

    node->data = o;
    return node;
}


void RelativePosition_free(RelativePosition_t* o){    
    o->data = NULL; 
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
    return count;
}

int ClusterCompair(const void* a, const void* b){
    RelativePosition_t* obj_a = *( (RelativePosition_t**) a);
    RelativePosition_t* obj_b = *( (RelativePosition_t**) b);

    if (obj_a->distance_to_origin < obj_b->distance_to_origin) {
        return -1; 
    }
    if (obj_a->distance_to_origin > obj_b->distance_to_origin) {
        return 1; 
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


cluster_element_t* createClusterElement(next_stop_t * data){
    cluster_element_t * node = (cluster_element_t*)malloc(sizeof(cluster_element_t));
    assert(node);

    node->data  = (next_stop_t*)malloc(sizeof(next_stop_t));    
    assert(node->data);

    memcpy(node->data, data, sizeof(next_stop_t));
    node->next = NULL;
    return node;
}

void pushClusterElement(cluster_element_t** head, next_stop_t * o ){
    
    cluster_element_t* node = createClusterElement(o);
    node->next = *head;
    *head = node;
}

cluster_list_t * createCluster(cluster_element_t * o){
    
    cluster_list_t* node = (cluster_list_t* )malloc( sizeof(cluster_list_t) );
    assert(node);

    node->data = o; // Poprawka: Po prostu przypisz wskaźnik, nie kopiuj danych.
    
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


void reverseClusterList(cluster_list_t** list_head_ref) {
    assert(&list_head_ref);

    cluster_list_t* current_cluster = *list_head_ref;
    while (current_cluster != NULL) {
        reverseElementsList(&(current_cluster->data));
        current_cluster = current_cluster->next;
    }

    cluster_list_t* head = *list_head_ref; // if empty, dont do anything
    
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



void calculateClustersPosition(ll_t** head, AMCOM_ObjectState * player, uint8_t count ){
    
    assert(&head);
    ll_t * current = *head;
   
    RelativePosition_t* node[count];
    
    for(uint8_t i = 0; i< count; ++i){
        
        node[i] = RelativePosition_Init(current->data);
        node[i]->distance_to_origin = GetDistance(player, node[i]->data); // set the distance from the first element in linked list 'head' 
        node[i]->data = node[i]->data;
        node[i]->angle_to_origin = GetAbsoluteAngle(player, node[i]->data);
        current = current->next;
        
    }

    printf("food position: received\n");
    if(ALGORITHM_DATA_LOG)  printClusterNode(node, count);
    
    sortClustersPosisions(node, count);

    printf("food position: sorted\n");
    if(ALGORITHM_DATA_LOG) printClusterNode(node, count);

    getClusters(node, count);
    
    // release memory
    for(uint8_t i = 0; i< count; ++i) RelativePosition_free(node[i]);
};


void NextStop_addClusters(cluster_list_t** head){
    uint8_t amount = GameDetails->next_stop_count;
    assert(amount != 0);

    uint8_t added_count = 0;
    cluster_list_t* current_cluster = *head;

     while (current_cluster != NULL && added_count < amount) {
        cluster_element_t* current_element = current_cluster->data;

        // Iteruj po elementach wewnątrz klastra
        while (current_element != NULL && added_count < amount) {
           
            GameDetails->next_stop[added_count] = *(current_element->data);
            
            added_count++;
            current_element = current_element->next;
        }
        current_cluster = current_cluster->next;
    }

}


void getClusters(RelativePosition_t* nodes[], uint8_t count) {
    cluster_list_t* list = NULL;
    uint8_t* visited = (uint8_t*)calloc(count, sizeof(uint8_t));
    
    assert(visited);

    for (uint8_t i = 0; i < count; ++i) {
        if (!visited[i]) {
            cluster_element_t* current_cluster_elements = NULL;

            
            next_stop_t p_i = { // new cluster element
                nodes[i]->distance_to_origin, 
                nodes[i]->angle_to_origin,
                nodes[i]->data
            };
            pushClusterElement(&current_cluster_elements, &p_i); // at least one element in the outer loop sets the cluser element. example: [ [cluster_element_1], [cluster_element_1,cluster_element_2]...]. If an entity hasn't got any 'close' elements, it stands alone as an element in clusterList
            visited[i] = 1;

            for (uint8_t j = i + 1; j < count; ++j) { // find new elements to add to cluster
                if (!visited[j]) {
                    float distance_diff = fabsf(nodes[i]->distance_to_origin - nodes[j]->distance_to_origin);
                    float angle_diff    = fabsf(nodes[i]->angle_to_origin - nodes[j]->angle_to_origin);

                    if (distance_diff < EPSILON_DISTANCE && angle_diff < EPSILON_ANGLE) { // decide if entity [j] is close to [i] is close
                        next_stop_t p_j = {
                            nodes[j]->distance_to_origin, 
                            nodes[j]->angle_to_origin,
                            nodes[j]->data
                        };
                        pushClusterElement(&current_cluster_elements, &p_j);
                        visited[j] = 1;
                    }
                }
            }
            pushCluster(&list, current_cluster_elements); // push ClusterElement into Cluster lust
        }
    }

    printf("food clusters discovered (amount)\n");
    if(ALGORITHM_DATA_LOG )   printCluster(&list);
    
    reverseClusterList(&list); // elements in stack, they're sorted and pushed, therefore reverse is needed.

    printf("food clusters sorted (amount)\n");

    if(ALGORITHM_DATA_LOG)  printCluster(&list);

    uint8_t amount_to_eat = (GameDetails->players_c == 1) ? GameDetails->transistor_c  : floorf(GameDetails->transistor_c -   3 *GameDetails->players_c );
    printf("\n%u\n", amount_to_eat);

    GameDetails->next_stop_count = amount_to_eat; 
    NextStop_Init();

    printf("next_stop\n");
    
    NextStop_addClusters(&list);
    
    printf("next_moves added from food clusters (amount to eat: %u)\n", amount_to_eat);
    if(ALGORITHM_DATA_LOG){
        for(uint8_t i = 0; i< amount_to_eat; ++i){
            printf("%d:(%f,%f)\n",i,GameDetails->next_stop[i].dist, GameDetails->next_stop[i].angle  );
        }
    }

    clearClusterList(&list);
    free(visited);
}

uint8_t foodReached(AMCOM_ObjectState * player){
    uint8_t food = GameDetails->current_stop;
    
    float xp = player->x, yp = player->y;
    float xf = GameDetails->next_stop[food].data->x, yf = GameDetails->next_stop[food].data->y;
    
    float distance = (float)sqrt(  
                (xp - xf ) * (xp - xf) + // (x_dest-x_origin)^2
                (yp - yf) * (yp - yf)   
                );

    if( distance <= EPSILON_DISTANCE){ // assumption that food is eaten
        
        GameDetails->next_stop_visited[food] = 1; // set the flag, have it being noted as eaten.
        return 1;               
    }
    return 0;
}




uint8_t inList(uint8_t element, const uint8_t arr[], uint8_t count){
    for(uint8_t i = 0; i< count; ++i){
        if (arr[i] == element){return 1;};
    }
    return 0;
}




uint8_t foodNext(const uint8_t excluded[], uint8_t count, AMCOM_ObjectState * player){ 
    uint8_t food_current = GameDetails->current_stop;
    
    for (uint8_t i = 0; i < GameDetails->next_stop_count; ++i){
   
         if (GameDetails->next_stop_visited[i] == 0 && !inList(i, excluded, count)) {
            
            GameDetails->current_stop = i;
            GameDetails->current_angle = GetAbsoluteAngle(player, GameDetails->next_stop[i].data);
            
            return 1; // Sukces, znaleziono nowy cel
        }
    }
    return 0;
}


AMCOM_ObjectState * PlayerSearchByHP(ll_t** head, AMCOM_ObjectState * my_player, uint8_t type){
    
    uint8_t my_health =  my_player->hp;

    AMCOM_ObjectState* theOne = NULL;
    ll_t* current = *head;
    if (current->next != NULL){ // if only one player, the while loop below causes crashes, hence the check
        if (type == 255){ // highest
            uint8_t higher = 0; 
            while(current!=NULL){

                if(current->data->hp > higher ){
                    higher = current->data->hp;
                    theOne = current->data;
                }
                current = current->next;
            }
        }else{
            uint8_t lower = my_health; 
            while(current!=NULL){

                if(current->data->hp < lower ){
                    lower = current->data->hp;
                    theOne = current->data;
                }
                current = current->next;
            }
        }
    
        if (my_player == theOne){  
            theOne = NULL;
        }
    }else{ 
        theOne = NULL;
    }
    return theOne;
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
            GameDetails->players_c = ptr->numberOfPlayers;

            toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &newGameResponse, sizeof(newGameResponse), buf);
            break;
        
        case AMCOM_GAME_OVER_REQUEST: //does not work
            if (REQ_RESP_LOG) printf("GAME_OVER.request. Responding with %s.\n", PLAYER_GONE);
            AMCOM_GameOverResponsePayload gameOverResponse;
            sprintf(gameOverResponse.endMessage, PLAYER_GONE);
            
            // free lists and clear the memory
            clearNode(&head_p);
            clearNode(&head_t);
            clearNode(&head_s);
            

            NextStop_Free();
            
            
            toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &gameOverResponse, sizeof(gameOverResponse), buf);
            break;
        

        case AMCOM_OBJECT_UPDATE_REQUEST:
            if (REQ_RESP_LOG) printf("OBJECTS.request. Reading the data\n");

            clearNode(&head_p); // players are sent every round, so clearing is nessecery 

            uint8_t objects_amount = packet->header.length / (uint8_t)sizeof(AMCOM_ObjectState);//  packet size-> 12
            AMCOM_ObjectState * current_object = (AMCOM_ObjectState *)packet->payload;
            
            fetchObjects(current_object, objects_amount); // handle packets with data
            
            break;  

        case AMCOM_MOVE_REQUEST:
            if (REQ_RESP_LOG) printf("MOVE.request. Responding with MOVING\n");
            
            AMCOM_MoveRequestPayload * gameRound = (AMCOM_MoveRequestPayload *)packet->payload; 

            AMCOM_ObjectState * player = findObject(0, GameDetails->my_player); // 0 means player type object
            assert(player);

            
            
            uint8_t blocked_food[GameDetails->next_stop_count]; // holds the blocked foods, used for skipping food entities in a list
            for(uint8_t i = 0; i< GameDetails->next_stop_count; ++i){
                blocked_food[i] = (uint8_t)BYPASS_ARG;
            }

            uint8_t blocked_food_index = 0;
            


            if ( gameRound->gameTime == 0 ){ // game initialization, run only once per new game
                printf("Round: %llu \n", gameRound->gameTime);
                
                GameDetails->transistor_c   = CountNodes(&head_t); // CountNodes asserts that they're set
                GameDetails->sparks_c       = CountNodes(&head_s);
                GameDetails->players_c      = CountNodes(&head_p);

                
                calculateClustersPosition(&head_t, player, GameDetails->transistor_c ); // get the elements to eat
                
                printf("cipa\n");
                foodNext(blocked_food, GameDetails->next_stop_count,player); 
                printf("dupa\n");

                printf("-----------------------------------\n");
                printf("Round: %llu: Foodie mode activated\n", gameRound->gameTime);
                printf("-----------------------------------\n");
                
            }

            if (GameDetails->current_stop == 255){ // predator mode
                AMCOM_ObjectState* theEnemy = NULL;
                theEnemy = PlayerSearchByHP(&head_p, player, 0); // find highest health
                if(theEnemy == NULL){
                    GameDetails->current_stop == 254;
                }
                GameDetails->current_angle = Move(player, theEnemy);
                


            }else if(GameDetails->current_stop == 254){
                
                GameDetails->current_angle = -PI + (float)rand() / RAND_MAX * (2 * PI);


            } else{ // foodie mode

                uint8_t is_reached = foodReached(player);
                uint8_t is_blocked = SparkInWay(player, GameDetails->next_stop[GameDetails->current_stop ].data);

                if (is_reached || is_blocked) {
                    if (is_reached) printf("Food (nr %u) found. Searching for another\n", GameDetails->current_stop);
                    if (is_blocked) printf("Food (#%u) blocked. Searching for another.\n", GameDetails->current_stop);
                    
                    
                    blocked_food[blocked_food_index++] = GameDetails->current_stop; // add the blocked element to the list of blocked elements
                   
                    uint8_t max_loops_iters = 0; // check for contuugnous iteration between constantly blocked foods.
                    while (max_loops_iters < 5) {
                     
                        if (foodNext(blocked_food, GameDetails->next_stop_count, player)) { // Znaleziono potencjalny nowy cel
                            if (SparkInWay(player, GameDetails->next_stop[GameDetails->current_stop].data)) { // food is blocked, find another one
                                
                                blocked_food[ blocked_food_index++] = GameDetails->current_stop;
                                
                                max_loops_iters++;
                                continue;
                            } else { // new, nonblocked food found
                                printf("Food (#%u) found. Setting the angle.\n", GameDetails->current_stop);
                                
                                blocked_food_index = 0;
                                for(uint8_t i = 0; i< GameDetails->next_stop_count; ++i){
                                    blocked_food[i] = (uint8_t)BYPASS_ARG;
                                }

                                break;
                            }
                        } else { // improvise mod
                            if(GameDetails->players_c == 1){
                                GameDetails->current_stop = 254;
                                printf("-----------------------------------\n");
                                printf("Round: %llu: Improvising mode activated\n", gameRound->gameTime);
                                printf("-----------------------------------\n");

                            }else{// no more food to eat, time for brutal anthropophagy
                                GameDetails->current_stop = 255;
                                printf("-----------------------------------\n");
                                printf("Round: %llu: Devourerer mode activated\n", gameRound->gameTime);
                                printf("-----------------------------------\n");

                            }

                            break;
                        }
                        max_loops_iters++;
                    }
                    // max while loops reached, changing state to killer
                   

                } 
            }


            AMCOM_MoveResponsePayload moveResponse;
            moveResponse.angle = GameDetails->current_angle;
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
