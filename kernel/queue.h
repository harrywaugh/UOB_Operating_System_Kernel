#include <stdlib.h>

typedef struct {
    struct node_t* previous;
    void*          item;
} node_t;

typedef struct {
    node_t* tail;
    node_t* head;
    int bytes;
    int length;
} queue_t;

queue_t *newQueue ( int bytesize );

void push ( queue_t *queue, void *p );
void peek ( queue_t *queue, void *p );
void pop  ( queue_t *queue, void *p );
