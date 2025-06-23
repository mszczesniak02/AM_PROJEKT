#ifndef OBJECTS
#define OBJECTS
#include <stdarg.h>
#include "node.h"



void OBJ_PrintObject(AMCOM_ObjectState * o);

void OBJ_PrintStacks(uint8_t count,...);

void OBJ_AssignObjects(void);

void OBJ_PacketState(void);

void OBJ_FillObjects(AMCOM_ObjectState * object, uint8_t object_amount);


#endif