#include "queue.h"

queue_t *newQueue( int bytesize )  {
    queue_t *nQueue = (queue_t *)malloc(sizeof(queue_t));
    *nQueue = (queue_t) {NULL, NULL, bytesize};
    return nQueue;
}

void push ( queue_t *queue, void *p )  {

    node_t *newNode       = (node_t *)malloc(sizeof(node_t *));       //Create new node
    newNode->item         = malloc((size_t)queue->bytes);
    *newNode              = (node_t){NULL, p};

    if (queue->tail == NULL)  {                                      //Set pointers to new node
        queue->head = newNode;
    } else {
        queue->tail->previous = newNode;
    }
    queue->tail = newNode;

    return;
}

void peek  ( queue_t *queue, void *p )  {
    p = queue->head;
}

void pop  ( queue_t *queue, void *p )  {
    p = queue->head;
    if (queue->head != NULL)  {
        queue->head = queue->head->previous;
    }
}
