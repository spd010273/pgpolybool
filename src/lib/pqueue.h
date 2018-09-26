#ifndef PQUEUE_H
#define PQUEUE_H

#include "polygon.h"

struct pqueue_node {
    struct sweep_event * element;
    struct pqueue_node * next;
};

struct pqueue_node * new_pqueue( struct sweep_event * );
struct sweep_event * pqueue_peek( struct pqueue_node ** );
struct sweep_event * pqueue_pop( struct pqueue_node ** );
void pqueue_push( struct pqueue_node **, struct sweep_event * );
bool pqueue_empty( struct pqueue_node ** );
int pqueue_size( struct pqueue_node ** );

struct deque_node {
    struct sweep_event * element;
    struct deque_node * next;
    struct deque_node * prev;
};

struct deque_head {
    struct deque_node * head;
    struct deque_node * tail;
    int length;
};

struct deque_head * new_deque( struct sweep_event * );
void deque_push_front( struct deque_head *, struct sweep_event * );
void deque_push_back( struct deque_head *, struct sweep_event * );
struct sweep_event * deque_pop_front( struct deque_head * );
struct sweep_event * deque_pop_back( struct deque_head * );
void free_deque( struct deque_head * );

#endif
