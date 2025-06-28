#ifndef STRUCTURES_H
#define STRUCTURES_H    

#include <winsock2.h>
#include <ws2tcpip.h>
#include <math.h>

#include <stdlib.h>
#include <stdio.h>

#include "amcom.h"
#include "amcom_packets.h"
// #include "logics.h"  // moved to the bottom, include being here doesnt let me compile


// #include "clusters.h"

#define PLAYER_NAME "mszczesniak"           
#define PLAYER_MSG  "intervention incoming."
#define PLAYER_GONE "don't have time for this."

#define GAME_SERVER "localhost"
#define GAME_SERVER_PORT "2001"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN



/**
 * \brief Stack like node, holds AMCOM_ObjectState data
 *
 */
typedef struct node{
    AMCOM_ObjectState * data;
    struct node * next;
}ll_t;


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
 * \brief  Temporary structure used for sorting the NextElement structure.
 */
typedef struct RelativePosition{
    AMCOM_ObjectState * data;
    float distance_to_origin;
    float angle_to_origin;

} RelativePosition_t;




/**
 * \brief  Stack structure with next_stop_t as data. Used for inner Cluster element
 */
typedef struct node2{
    next_stop_t * data;
    struct node2 * next;
}cluster_element_t;



/**
 * \brief  Stack structure with cluster_element_t as data. Outer list, that holds innner list
 */
typedef struct node1 {
    cluster_element_t * data;
    struct node1 * next;
}cluster_list_t ;



#include "logics.h" 

#endif