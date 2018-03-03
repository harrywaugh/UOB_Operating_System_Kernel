#include "queue.h"
#include <string.h>

queue_t *newQueue( int bytesize )  {
    queue_t nQueue;
    memset( &nQueue, 0, sizeof( queue_t ) );

    nQueue.item = bytesize;
    nQueue.length = 0;
    return &nQueue;
}

void push ( queue_t *queue, void *p )  {

    node_t newNode;                                   //Create new node
    memset( &newNode, 0, sizeof( queue_t ) );

    newNode.item = p;


    if (queue->length == 0)  {                                      //Set pointers to new node
        queue->head = &newNode;
    } else {
        queue->tail->previous = &newNode;
    }
    queue->tail = &newNode;

    return;
}

void peek  ( queue_t *queue, void *p )  {
    p = queue->head->item;
}

void pop  ( queue_t *queue, void *p )  {
    p = queue->head->item;
    if (queue->length != 0)  {
        queue->head = queue->head->previous;
    }
}
