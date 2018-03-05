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

    if (queue->head == NULL)  {                                     //If queue is empty: Add the node
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        node_t *currentNode = queue->head;
        if (currentNode->pcb->priority < pcb->priority)  {         //If new pcb is higher priority, then add to front
            newNode->previous = currentNode;
            queue->head = newNode;
        }
        while ((currentNode->previous != NULL) && (&(&(currentNode->previous)->pcb)->priority >= pcb->priority))  { //Cycle back to find slot in point
            currentNode = currentNode->previous;
        }
        if (currentNode->previous == NULL)  queue->tail = newNode;                  //If new pcb has least priority, add to back
        else                                newNode->previous = currentNode->previous;    //Else slot it in a gap.
        currentNode->previous = newNode;

    }

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
