#include "pqueue.h"

struct pqueue_node * new_pqueue( struct sweep_event * data )
{
    struct pqueue_node * node = NULL;

    node = ( struct pqueue_node * ) palloc0( sizeof( struct pqueue_node ) );

    node->next     = NULL;
    node->element  = data;

    return node;
}

struct sweep_event * pqueue_peek( struct pqueue_node ** head )
{
    return (*head)->element;
}

struct sweep_event * pqueue_pop( struct pqueue_node ** head )
{
    struct sweep_event * data = NULL;
    struct pqueue_node * temp = NULL;

    temp    = *head;
    (*head) = (*head)->next;
    data    = temp->element;

    pfree( temp );

    return data;
}

void pqueue_push( struct pqueue_node ** head, struct sweep_event * data )
{
    struct pqueue_node * start = NULL;
    struct pqueue_node * temp  = NULL;

    start = (*head);
    temp = new_pqueue( data );

    if( sweep_event_comp( data, (*head)->element )  )
    {
        temp->next = *head;
        (*head) = temp;
    }
    else
    {
        while( start->next != NULL && sweep_event_comp( start->next->element, data ) )
        {
            start = start->next;
        }

        temp->next = start->next;
        start->next = temp;
    }

    return;
}

bool pqueue_empty( struct pqueue_node ** head )
{
    return (*head) == NULL;
}

int pqueue_size( struct pqueue_node ** head )
{
    int                  ret  = 0;
    struct pqueue_node * temp = NULL;

    if( pqueue_empty( head ) )
    {
        return 0;
    }

    temp = (*head);

    while( temp->next != NULL )
    {
        ret++;
    }

    return ret;
}

struct deque_head * new_deque( struct sweep_event * data )
{
    struct deque_head * deque = NULL;
    struct deque_node * node = NULL;

    if( data == NULL )
    {
        return NULL;
    }

    node = ( struct deque_node * ) palloc0( sizeof( struct deque_node ) );
    deque = ( struct deque_head * ) palloc0( sizeof( struct deque_head ) );

    node->element = data;
    node->next = node;
    node->prev = node;
    deque->head = node;
    deque->tail = node;
    deque->length = 1;
    return deque;
}

void deque_push_front( struct deque_head * deque, struct sweep_event * e )
{
    struct deque_node * node = NULL;
    if( deque == NULL || e == NULL )
    {
        return;
    }

    node = ( struct deque_node * ) palloc0( sizeof( struct deque_node ) );
    node->element = e;
    node->prev = node;
    deque->head->prev = node;
    node -> next = deque->head;
    deque->head = node;
    deque->length++;
    return;
}

void deque_push_back( struct deque_head * deque, struct sweep_event * e )
{
    struct deque_node * node = NULL;
    if( deque == NULL || e == NULL )
    {
        return;
    }

    node = ( struct deque_node * ) palloc0( sizeof( struct deque_node * ) );
    node->element = e;
    node->prev = deque->tail;
    node->next = node;
    deque->tail = node;
    deque->length++;
    return;
}

struct sweep_event * deque_pop_front( struct deque_head * deque )
{
    struct sweep_event * e = NULL;
    struct deque_node * temp = NULL;
    if( deque == NULL )
    {
        return NULL;
    }

    temp = deque->head;
    e = temp->element;
    deque->head = temp->next;
    deque->head->prev = deque->head;
    deque->length--;
    pfree( temp );
    return e;
}

struct sweep_event * deque_pop_back( struct deque_head * deque )
{
    struct sweep_event * e = NULL;
    struct deque_node * temp = NULL;

    if( deque == NULL )
    {
        return NULL;
    }

    temp = deque->tail;
    e = temp->element;
    deque->tail = temp->prev;
    deque->tail->next = deque->tail;
    deque->length--;
    pfree( temp );
    return e;
}

void free_deque( struct deque_head * deque )
{
    struct deque_node * node = NULL;

    node = deque->head;

    while( node != deque->tail )
    {
        node = node->next;
        pfree( node->prev );
    }

    pfree( node );
    pfree( deque );
    return;
}
