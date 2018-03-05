#include "queue.h"
#include "string.h"
#include <stdio.h>

queue_t *newQueue()  {
    queue_t *nQueue = (queue_t *)malloc(sizeof(queue_t));

    nQueue->head = (node_t *)malloc(sizeof(node_t));
    nQueue->tail = (node_t *)malloc(sizeof(node_t));

    *nQueue = (queue_t) {NULL, NULL};
    return nQueue;
}

void push ( queue_t *queue, pcb_t *pcb )  {

    node_t *newNode       = (node_t *)malloc(sizeof(node_t));       //Create new node
    newNode->pcb          = (pcb_t *)malloc(sizeof(pcb_t));
    memcpy(newNode->pcb, pcb, sizeof(pcb_t));
    newNode->previous    = NULL;

    if (queue->tail == NULL)  {                                      //Set pointers to new node
        queue->head = newNode;
    } else {
        queue->tail->previous = newNode;
    }
    queue->tail = newNode;

    return;
}

void peek ( queue_t *queue, pcb_t *pcb )  {
    if (queue->head != NULL){
        memcpy(pcb, queue->head->pcb, sizeof(pcb_t));
    }
}

void pop ( queue_t *queue, pcb_t *pcb )  {
    if (queue->head != NULL)  {
        memcpy(pcb, queue->head->pcb, sizeof(pcb_t));
        node_t *oldHeadNode = queue->head;
        queue->head = oldHeadNode->previous;
        free(oldHeadNode);
    }
}
