#include "alg.h"

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

}

void ALG_Transpose(AMCOM_ObjectState * object){ //transpose from -500/500 to 0/1000
    object->x += 500.0f;
    object->y += 500.0f;
}