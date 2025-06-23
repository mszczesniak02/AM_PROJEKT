#ifndef GAME_DETAILS
#define GAME_DETAILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "amcom.h"
#include "amcom_packets.h"


#define AMCOM_MAX_OBJECTS 32
#define AMCOM_MAX_PLAYERS 8
#define AMCOM_MAX_STATIC_OBJECTS 24

/*
    Game object that allows to not interfere with already established TCP connection code
*/
typedef struct {

    AMCOM_ObjectState objects[AMCOM_MAX_OBJECTS];
  
    
    Node_t * head_p;
    Node_t * head_t;
    Node_t * head_s;
    Node_t * head_g;
    
    uint8_t players_c, transistors_c, sparks_c, glue_c; // x_c -> count

    uint8_t objects_total;
    uint8_t players_total;
    uint8_t player_num;
        
}GameDetails_t;

void GameDetailsInit(GameDetails_t * details);
void FillGamedata(AMCOM_NewGameRequestPayload * game_data);


#endif