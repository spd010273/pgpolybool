#ifndef RBTREE_H
#define RBTREE_H

#ifndef RBTREE_TEST
#include "util.h"

#define _RBTREE_ALLOC(size) palloc0(size)
#define _RBTREE_REALLOC(ptr,size) repalloc(ptr,size)
#define _RBTREE_FREE(size) pfree(size)
#define _RBTREE_LOG(msg,args...) elog(DEBUG1,msg,args)
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>

#define _RBTREE_ALLOC(size) calloc(1,size)
#define _RBTREE_FREE(ptr) free(ptr)
#define _RBTREE_LOG(msg,args...) rbtree_log(msg,args)
#endif

#define is_red(x) ((x != NULL && x->red))
#define rbtree_stack_push(sp,x) ((x)->next = (sp), (sp) = (x))
#define rbtree_stack_pop(sp) ((sp) = (sp)->next)
#define rbtree_stack_top(sp) (sp)
#define rbtree_empty(x) ((x==NULL || x->root==NULL || rbtree_size(x) == 0))

#define RBTREE_DEBUG DEBUG

struct rbtree_node {
    void * data;
    bool red;
    struct rbtree_node * left;
    struct rbtree_node * right;
    struct rbtree_node * parent;
    struct rbtree_node * next;    /* for iterator stack */
    unsigned int subcount;
};

struct rbtree {
    struct rbtree_node * root;
    struct rbtree_node * rstack;
    struct rbtree_node * iter;
    bool (*compare)( void *, void * );
    bool (*equal)( void *, void * );
    unsigned int size;
#ifdef RBTREE_DEBUG
    void (*debug)( void * );
    char * (*pretty_print)( void *);
#endif // RBTREE_DEBUG
};

struct rbtree * new_rbtree(
    bool (*)( void *, void * ), // compare
    bool (*)( void *, void * )  // equal
);
struct rbtree_node * new_rbtree_node( void * );
void free_rbtree( struct rbtree * );
void _delete_tree( struct rbtree_node * );
void rbtree_destroy( struct rbtree * );
unsigned int _count_nodes( struct rbtree_node * );
unsigned int rbtree_size( struct rbtree * );
struct rbtree_node * _insert(
    struct rbtree *,
    struct rbtree_node *,
    struct rbtree_node *,
    void *
);
void rbtree_insert( struct rbtree *, void * );
void * rbtree_peek_position( struct rbtree *, unsigned int );
unsigned int rbtree_get_position( struct rbtree *, void * );
struct rbtree_node * _delete( struct rbtree *, struct rbtree_node *, void * );
void rbtree_delete( struct rbtree *, void * );
struct rbtree_node * rbtree_search( struct rbtree *, void * );
void _traverse_tree( struct rbtree_node *, void (*)(void *) );
void rbtree_foreach( struct rbtree *, void (*)(void *) );
void * rbtree_pop( struct rbtree * );
void rbtree_iter_begin( struct rbtree * );
struct rbtree_node * rbtree_iter_next( struct rbtree * );
void rbtree_iter_reset( struct rbtree * );
void color_flip( struct rbtree_node * );
struct rbtree_node * rotate_left( struct rbtree_node * );
struct rbtree_node * rotate_right( struct rbtree_node * );
struct rbtree_node * find_min( struct rbtree_node * );
void rbtree_delete_min( struct rbtree * );
struct rbtree_node * del_min( struct rbtree *, struct rbtree_node * );
struct rbtree_node * find_max( struct rbtree_node * );
struct rbtree_node * del_max( struct rbtree *, struct rbtree_node * );
struct rbtree_node * move_red_left( struct rbtree_node * );
struct rbtree_node * move_red_right( struct rbtree_node * );
struct rbtree_node * fix_up( struct rbtree_node * );

#ifdef RBTREE_DEBUG
void rbtree_setup_debug( struct rbtree *, void (*)( void * ) );
void rbtree_setup_pretty_print( struct rbtree *, char *(*)(void *) );
void rbtree_dummy_debug( void * );
void rbtree_debug( struct rbtree * );
#endif // RBTREE_DEBUG

#ifdef RBTREE_TEST
extern void rbtree_log( char *, ... ) __attribute__ ((format (gnu_printf, 1, 2 )));
#endif // RBTREE_TEST
#endif  // RBTREE_H
