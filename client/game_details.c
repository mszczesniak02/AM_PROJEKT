#include "game_details.h"

/*
    Init game structre, 
*/
void GameDetailsInit(GameDetails_t * details){


    details->objects_total  = 0;    // gówno
    details->player_num     = 0;    // gówno
    details->players_total  = 0;    // gówno 

    details->glue_c         = 0;    // stores the incoming packet's data count   
    details->transistors_c  = 0;    // stores the incoming packet's data count  
    details->sparks_c       = 0;    // stores the incoming packet's data count  
    details->players_c      = 0;    // stores the incoming packet's data count  

    details->head_p = NULL; // stack, holds the objects 
    details->head_t = NULL; // stack, holds the objects 
    details->head_s = NULL; // stack, holds the objects 
    details->head_g = NULL; // stack, holds the objects 

    memset(details->objects, 0, sizeof(details->objects));
};

extern GameDetails_t * GameDetails;

void FillGamedata(AMCOM_NewGameRequestPayload * game_data){
    GameDetails->players_total = game_data->numberOfPlayers;
    GameDetails->player_num = game_data->playerNumber;
}