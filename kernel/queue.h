#include <stdlib.h>
#include <stdbool.h>


struct node_t {
    struct node_t* previous;
    void *item;
};
typedef struct node_t node_t;

typedef struct {
    node_t* tail;
    node_t* head;
    int b;
} queue_t;


queue_t *newQueue (int b);

bool isEmpty ( queue_t *queue );
void push ( queue_t *queue, void *p );
void prioritypush ( queue_t *queue, void *p );
void peek ( queue_t *queue, void *p );
bool pop  ( queue_t *queue, void *p );
