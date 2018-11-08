/*
 * dlpq (Double-Linked Priority Queue) is released under the MIT License
 *
 *    Copyright 2018 Chris Autry
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy of
 *    this software and associated documentation files (the "Software"), to deal in
 *    the Software without restriction, including without limitation the rights to
 *    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *    the Software, and to permit persons to whom the Software is furnished to do so,
 *    subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "dlpq.h"

/*
 * Queue structure is as follows
 *
 *                 data n        data n-1      data n-2          data 0
 *                   ^             ^             ^                 ^
 *                   |             |             |                 |
 *                 value         value         value             value
 *                   |             |             |                 |
 *                 ------        ------        ------            ------
 * <-dlpq_unshift- |Node|-next-> |Node|-next-> |Node|-next-> ... |Node| -next-> NULL
 *    NULL <- prev-|  n | <-prev-| n-1| <-prev-| n-2| ... <-prev-|zero| -dlpq_pop->
 *                 ------        ------        ------            ------
 *                   ^                                              ^
 *                   |                                              |
 *                 frist                                           last
 *                   |                ----------------              |
 *                   |--------------- | Head         | -------------|
 *                                    | size = n + 1 |
 *                                    ----------------
 *
 * The constructor takes a function pointer capable of comparing the void * data values of nodes
 * for example, a simple priority queue could be implemented using the function:
 *
 * bool compare( void * a, void * b )
 * {
 *     return *((int *) a) < *((int *) b);
 * }
 *
 * Where the elements are int * cast to void * prior to addition to the queue
 * This would resul in a queue ordered as follows:
 *
 *   POS   VAL
 *  last   0
 *     1   1
 *     2   2
 *     .
 *     .
 *     .
 * first   100000
 *
 */

bool default_compare_function( void * a, void * b )
{
    if( a < b )
    {
        return true;
    }

    return false;
}

struct dlpq * new_dlpq( bool (*compare_function)( void *, void * ) )
{
    struct dlpq * head = NULL;

    head = ( struct dlpq * ) _ALLOC( sizeof( struct dlpq ) );

    if( head == NULL )
    {
        _LOG( "Failed to allocate memory for DLPQ: head returned %p", head );
        return NULL;
    }

    head->first   = NULL;
    head->last    = NULL;
    head->size    = 0;

    if( compare_function == NULL )
    {
        head->compare = &default_compare_function;
    }
    else
    {
        head->compare = compare_function;
    }

    return head;
}

inline unsigned int dlpq_size( struct dlpq * head )
{
    return ( head == NULL ) ? 0 : head->size;
}

inline bool dlpq_empty( struct dlpq * head )
{
    return ( head == NULL || head->size == 0 ) ? true : false;
}

unsigned int dlpq_get_position( struct dlpq * head, void * data )
{ //naive function
    unsigned int       position = 0;
    struct dlpq_node * node     = NULL;

    if( head == NULL )
    {
        return 0;
    }

    node = head->last;

    while( node != NULL )
    {
        if( node->value == data )
        {
            return position;
        }

        node = node->prev;
        position++;
    }

    return 0;
}

void * dlpq_peek_position( struct dlpq * head, unsigned int position )
{
    unsigned int       i    = 0;
    struct dlpq_node * node = NULL;

    if( head == NULL || position >= head->size )
    {
        if( head != NULL && position >= head->size )
        {
            _LOG(
                "position %d is out of bound of dlpq sized %d",
                position,
                head->size
            );
        }

        return NULL;
    }

    node = head->last;

    for( i = 0; i < position; i++ )
    {
        node = node->prev;
    }

    return node->value;
}

void dlpq_remove( struct dlpq * head, void * data )
{
    struct dlpq_node * node = NULL;
    struct dlpq_node * temp = NULL;

    if( head == NULL || data == NULL )
    {
        return;
    }

    if( dlpq_empty( head ) )
    {
        return;
    }

    if( head->size == 1 )
    {
        _FREE( head->first );
        head->size = 0;
        head->first = NULL;
        head->last = NULL;
        return;
    }

    if( data == head->first->value )
    {
        node = head->first;
        head->first = node->next;
        head->first->prev = NULL;

        _FREE( node );
        head->size--;
        return;
    }
    else if( data == head->last->value )
    {
        node = head->last;
        head->last = node->prev;
        head->last->next = NULL;
        _FREE( node );
        head->size--;
        return;
    }

    node = head->first;

    while( node != NULL )
    {
        if( node->value == data )
        {
            temp = node->next;
            node->next->prev = node->prev;
            node->prev->next = temp;
            _FREE( node );
            head->size--;
            return;
        }

        node = node->next;
    }

    return;
}

void dlpq_push( struct dlpq * head, void * data )
{
    struct dlpq_node * new_node   = NULL;
    struct dlpq_node * last_node  = NULL;
    struct dlpq_node * start_node = NULL;

    if( head == NULL )
    {
        _LOG( "DLPQ head is %p", head );
        return;
    }

    if( data == NULL )
    {
        _LOG( "Passed data is %p", data );
        return;
    }

    new_node = ( struct dlpq_node * ) _ALLOC( sizeof( struct dlpq_node ) );

    if( new_node == NULL )
    {
        _LOG( "NEW DLPQ node failed to allocate, got %p", new_node );
        return;
    }

    new_node->next  = NULL;
    new_node->prev  = NULL;
    new_node->value = data;

    if( dlpq_empty( head ) )
    {
        head->first = new_node;
        head->last  = new_node;
        head->size  = 1;
        return;
    }

    start_node = head->first;
    last_node  = start_node;
    head->size++;

    // Handle possible head replacement
    if( head->compare( data, start_node->value ) )
    {
        head->first      = new_node;
        new_node->next   = start_node;
        start_node->prev = new_node;
    }
    else
    {
        // Locate insertion point
        while( start_node != NULL )
        {
            if( head->compare( data, start_node->value ) )
            {
                last_node->next  = new_node;
                new_node->prev   = last_node;
                new_node->next   = start_node;
                start_node->prev = new_node;
                break;
            }

            last_node  = start_node;
            start_node = start_node->next;

            // Handle tail replacement
            if( start_node == NULL )
            {
                new_node->prev   = head->last;
                head->last->next = new_node;
                head->last       = new_node;
                new_node->next   = NULL;
                break;
            }
        }
    }

    return;
}

inline void * dlpq_pop( struct dlpq * head )
{
    // Take an element from head->last (the head of the queue)
    struct dlpq_node * temp = NULL;
    void *             data = NULL;

    if( dlpq_empty( head ) )
    {
        return NULL;
    }

    head->size--;

    temp       = head->last;
    data       = temp->value;
    head->last = temp->prev;

    if( head->last != NULL )
    {
        head->last->next = NULL;
    }

    _FREE( temp );

    return data;
}

inline void * dlpq_unshift( struct dlpq * head )
{
    // Take an element head->first ( the tail of the queue )
    struct dlpq_node * temp = NULL;
    void *             data = NULL;

    if( dlpq_empty( head ) )
    {
        return NULL;
    }

    head->size--;

    temp             = head->first;
    data             = temp->value;
    head->first      = temp->next;

    if( head->first != NULL )
    {
        head->first->prev = NULL;
    }

    _FREE( temp );
    temp = NULL;

    return data;
}

void free_dlpq( struct dlpq ** head )
{
    struct dlpq_node * next_node = NULL;

    if( *head == NULL )
    {
        return;
    }

    if( dlpq_empty( *head ) && (*head) != NULL )
    {
        _FREE( *head );
        return;
    }

    next_node = (*head)->last;

    while( next_node != (*head)->first )
    {
        next_node = next_node->prev;
        _FREE( next_node->next );
    }

    if( next_node != NULL )
    {
        _FREE( next_node );
    }

    _FREE( (*head) );
    (*head) = NULL;
    return;
}

#ifdef DLPQ_DEBUG
void _dlpq_setup_debug( struct dlpq * head, void (*debug)( void * ) )
{
    if( head == NULL || debug == NULL || debug == NULL )
    {
        return;
    }

    head->__dump_function = debug;

    return;
}

void _dlpq_debug( struct dlpq * head )
{
    struct dlpq_node * temp  = NULL;
    unsigned int       index = 0;

    if( head == NULL )
    {
        _LOG( "dlpq head is %p\n", head );
    }

    temp = head->last;
    _LOG(
        "HEAD: %p, first: %p last: %p size: %u",
        head,
        head->first,
        head->last,
        head->size
    );
    while( temp != NULL )
    {
/*
        _LOG(
            "Node #%u: %p prev %p next %p DATA: %p",
            index,
            temp,
            temp->prev,
            temp->next,
            temp->value
        );
  */
        if( head->__dump_function != NULL )
        {
            head->__dump_function( temp->value );
        }

        temp = temp->prev;
        index++;
    }

    return;
}
#endif // DLPQ_DEBUG
