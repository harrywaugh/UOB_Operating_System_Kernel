#include <stdlib.h>

typedef struct {
    struct node_t* previous;
    int          id;
} node_t;

typedef struct {
    node_t* tail;
    node_t* head;
} queue_t;

queue_t *newQueue ();

void push ( queue_t *queue, int i);
int peek ( queue_t *queue );
int pop  ( queue_t *queue );
