#include "queue.h"
#include "string.h"
#include <stdio.h>

queue_t *newQueue(int b)  {
    queue_t *nQueue = (queue_t *)malloc(sizeof(queue_t));

    nQueue->head = (void *)malloc(sizeof(node_t));
    nQueue->tail = (void *)malloc(sizeof(node_t));

    *nQueue = (queue_t) {NULL, NULL, b};
    return nQueue;
}

void push ( queue_t *queue, void *p )  {

    node_t *newNode       = (node_t *)malloc(sizeof(node_t));       //Create new node
    newNode->item          = malloc((size_t)queue->b);

    memcpy(newNode->item, p, (size_t)queue->b);
    newNode->previous    = NULL;

    if (queue->head == NULL)  {                                      //Set pointers to new node
        queue->head = newNode;
    } else {
        queue->tail->previous = newNode;
    }
    queue->tail = newNode;

    return;
}

// void prioritypush ( queue_t *queue, void *p )  {
//
//     node_t *newNode       = (node_t *)malloc(sizeof(node_t));       //Create new node
//     newNode->item          = malloc((size_t)queue->b);
//     memcpy(newNode->item, p, (size_t)queue->b) ;
//     newNode->previous    = NULL;
//
//     if (queue->head == NULL)  {                                     //If queue is empty: Add the node
//         queue->head = newNode;
//         queue->tail = newNode;
//         return;
//     }
//     node_t *currentNode = queue->head;
//     if (currentNode->pcb->priority < pcb->priority)  {         //If new pcb is higher priority, then add to front
//         newNode->previous = currentNode;
//         queue->head = newNode;
//         return;
//     }
//     while ((currentNode->previous != NULL) && (currentNode->previous->pcb->priority >= pcb->priority))  { //Cycle back to find slot in point
//         currentNode = currentNode->previous;
//     }
//     if (currentNode->previous == NULL)  queue->tail = newNode;                  //If new pcb has least priority, add to back
//     else                                newNode->previous = currentNode->previous;    //Else slot it in a gap.
//     currentNode->previous = newNode;
//     return;
// }

void peek ( queue_t *queue, void *p )  {
    if (queue->head != NULL){
        memcpy(p, queue->head->item, (size_t)queue->b);
    }
}

void pop ( queue_t *queue, void *p )  {
    if (queue->head != NULL)  {
        memcpy(p, queue->head->item, (size_t)queue->b);
        node_t *oldHeadNode = queue->head;
        queue->head = oldHeadNode->previous;
        free(oldHeadNode->item);
        free(oldHeadNode);
    }
    if(queue->head == NULL)  {
        queue->tail = NULL;
    }
}

bool isEmpty( queue_t *queue )  {
    if ( queue->head == NULL)  return true;
    return false;
}
