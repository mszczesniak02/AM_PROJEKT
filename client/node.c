#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"


Node_t* createNode(AMCOM_ObjectState * object){
    Node_t * n = (Node_t *)malloc(sizeof(Node_t));
    if(n == NULL){
        return NULL;
    }
    n->data = object;
    n->next = NULL;
    return n;
}

void pushNode(Node_t ** head, AMCOM_ObjectState * object){

    Node_t* n = createNode(object);

    if (*head == NULL){
        *head = n;
    }else{
        n->next = (*head);
        *head = n;

    }
}


void deleteHeadNode(Node_t ** head){
    Node_t* t = *head;
    *head = (*head)->next;
    free(t);
}

void deleteAllNode(Node_t **head) {
    Node_t *current = *head;
    Node_t *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL; // <- to jest kluczowe!
}






void printStack(Node_t** stack){
    Node_t* temp = *stack;
    while (temp != NULL) {
        OBJ_PrintObject(temp->data);
        temp = temp->next;
    }
    printf("\n");
}

