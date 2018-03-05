#include <stdlib.h>
#include "hilevel.h"


typedef struct {
    struct node_t* previous;
    pcb_t          *pcb;
} node_t;

typedef struct {
    node_t* tail;
    node_t* head;
} queue_t;

queue_t *newQueue ();

void push ( queue_t *queue, pcb_t *pcb );
void peek ( queue_t *queue, pcb_t *pcb );
void pop  ( queue_t *queue, pcb_t *pcb );
