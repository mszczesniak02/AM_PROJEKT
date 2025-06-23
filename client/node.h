#ifndef NODE
#define NODE

#include <stdio.h>
#include "amcom.h"
#include "amcom_packets.h"
#include "objects.h"



/*
    Stack structure, used for storage of objects and players
*/
typedef struct node{
    AMCOM_ObjectState * data;
    struct node* next;
}Node_t;


/*
    Create a new Node, allocates the memory and returnes with assigned object to it
*/
Node_t* createNode(AMCOM_ObjectState * object);

/*
    Push object into the structre, head being the top element
*/
void pushNode(Node_t ** head, AMCOM_ObjectState * object);      

/*
    Pops and returns the element
*/
Node_t * popNode(Node_t ** head);

/*
    Delete the top node and free the memory
*/
void deleteHeadNode(Node_t ** head);

/*
    Delete all stack/node and free memory
*/
void deleteAllNode(Node_t **head);




/*
    quick print for debug
*/
void printStack(Node_t** stack);

#endif