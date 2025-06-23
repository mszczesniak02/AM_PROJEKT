#ifndef ALG
#define ALG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "node.h"
#include "amcom.h"
#include "amcom_packets.h"

float ALG_GetDistance(AMCOM_ObjectState * object_origin,AMCOM_ObjectState * object_dest );

float ALG_GetRelativeAngle(AMCOM_ObjectState * object_origin, AMCOM_ObjectState * object_dest);

void ALG_Transpose(AMCOM_ObjectState * object);


#endif