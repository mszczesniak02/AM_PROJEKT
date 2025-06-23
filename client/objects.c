#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "game_details.h"

extern GameDetails_t* GameDetails;

void OBJ_FillObjects(AMCOM_ObjectState * object, uint8_t object_amount){

    for(uint8_t i = 0 ; i<object_amount; ++i){
        GameDetails_t * p = GameDetails;
        p->objects[i] = object[i];

        switch(p->objects[i].objectType){
            case 0: // players
                 p->players_c++;
                pushNode(&p->head_p, &p->objects[i]);
                break;
            case 1:
                p->transistors_c++;
                pushNode(&p->head_t, &p->objects[i]);
                break;
            case 2:
                p->sparks_c++;
                pushNode(&p->head_s, &p->objects[i]);
                break;
            case 3:
                p->glue_c++;
                pushNode(&p->head_g, &p->objects[i]);
                break;
        } 
    }
    GameDetails->objects_total = object_amount ;
}


void OBJ_PacketState(void){
    GameDetails_t * p = GameDetails;
    
    printf("PLAYERS: (%u):\n", p->players_c );
    printf("TRANS:   (%u):\n", p->transistors_c);
    printf("sparks:  (%u):\n",p->sparks_c);
    printf("glues:   (%u):\n", p->glue_c);
}


void OBJ_AssignObjects(void){
    GameDetails_t * p = GameDetails; 
    // do not clear the stacks, they keep the objects during the whole game.


    GameDetails->players_c = 0;
    GameDetails->transistors_c = 0;
    GameDetails->sparks_c = 0;
    GameDetails->glue_c= 0;
   
    for (uint8_t i = 0; i < p->objects_total; ++i){
       
        switch(p->objects[i].objectType){
            case 0: // players
                 p->players_c++;
                pushNode(&p->head_p, &p->objects[i]);
                break;
            case 1:
                p->transistors_c++;
                pushNode(&p->head_t, &p->objects[i]);
                break;
            case 2:
                p->sparks_c++;
                pushNode(&p->head_s, &p->objects[i]);
                break;
            case 3:
                p->glue_c++;
                pushNode(&p->head_g, &p->objects[i]);
                break;
        } 
    }
}


void OBJ_PrintObject(AMCOM_ObjectState * o){
    printf("\nO.number: %ll u \n", o->objectNo );
    printf("O.type:   %u \n",   o->objectType );
    printf("O.HP:     %d \n",   o->hp);
    printf("O.POSX:   %f \n",   o->x);
    printf("O.POSY:   %f \n\n", o->y);
}


