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

#ifndef DLPQ_H
#define DLPQ_H

#include "postgres.h"

#define DLPQ_DEBUG DEBUG

#define _ALLOC(size) palloc0(size)
#define _FREE(ptr) pfree(ptr)
#define _LOG(msg,args...) elog(DEBUG1,msg,args)

struct dlpq_node {
    void * value;
    struct dlpq_node * next;
    struct dlpq_node * prev;
};

struct dlpq {
    struct dlpq_node * first; // list of tree roots
    struct dlpq_node * last;
    unsigned int size;
    bool (*compare)( void *, void * );
#ifdef DLPQ_DEBUG
    void (*__dump_function)( void * ); // Debug object dumper
#endif // DLPQ_DEBUG
};

extern bool default_compare_function( void *, void * );
extern struct dlpq * new_dlpq( bool (*)( void *, void * ) );
extern inline unsigned int dlpq_size( struct dlpq * );
extern inline bool dlpq_empty( struct dlpq * );
extern unsigned int dlpq_get_position( struct dlpq *, void * );
extern void * dlpq_peek_position( struct dlpq *, unsigned int );
extern void dlpq_remove( struct dlpq *, void * );
extern void dlpq_push( struct dlpq * head, void * data );
extern inline void * dlpq_pop( struct dlpq * );
extern inline void * dlpq_unshift( struct dlpq * );
extern void free_dlpq( struct dlpq ** );
#ifdef DLPQ_DEBUG
extern void _dlpq_debug( struct dlpq * );
extern void _dlpq_setup_debug( struct dlpq *, void (*)( void * ) );
#endif // DLPQ_DEBUG
#endif // DLPQ_H
