#include <stdio.h>
#include <stdlib.h>
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

Node_t * popNode(Node_t ** head){
    Node_t* t = *head;
    *head = (*head)->next;
    return t;
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

void deleteAllNodes(uint8_t count,...){
    va_list args;
    va_start(args, count);
    
    for (int i = 0; i < count; ++i) {
        Node_t **head = va_arg(args, Node_t **);
        deleteAllNode(head);
    }
    va_end(args);
};




void printStack(Node_t** stack){
    Node_t* temp = *stack;
    while (temp != NULL) {
        OBJ_PrintObject(temp->data);
        temp = temp->next;
    }
    printf("\n");
}

