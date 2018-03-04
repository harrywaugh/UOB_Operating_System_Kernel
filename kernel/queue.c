#include "queue.h"
#include "hilevel.h"
#include "string.h"
#include <stdio.h>

queue_t *newQueue()  {
    queue_t *nQueue = (queue_t *)malloc(sizeof(queue_t));

    nQueue->head = (node_t *)malloc(sizeof(node_t *));
    nQueue->tail = (node_t *)malloc(sizeof(node_t *));

    *nQueue = (queue_t) {NULL, NULL};
    return nQueue;
}

void push ( queue_t *queue, int i )  {

    node_t *newNode       = (node_t *)malloc(sizeof(node_t *));       //Create new node
    newNode->id           = i;
    newNode->previous    = NULL;

    if (queue->tail == NULL)  {                                      //Set pointers to new node
        queue->head = newNode;
    } else {
        queue->tail->previous = newNode;
    }
    queue->tail = newNode;

    return;
}

int peek  ( queue_t *queue )  {
    if (queue->head != NULL){
        return queue->head->id;
    }
    return -1;
}

int pop  ( queue_t *queue )  {
    if (queue->head != NULL)  {
        int i = queue->head->id;
        node_t *oldHeadNode = queue->head;
        queue->head = oldHeadNode->previous;
        free(oldHeadNode);
        return i;
    }
    return -1;
}
