/*------------------------------------------------------------------------------
 * rbtree.c
 *     Source file listing interface and private functions for
 *     Red-Black Binary Search Tree
 *     This is a basic RB-BST with augmentations for maintaining node counts,
 *     node parents, and utilizes a slab allocator.
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      rbtree.c
 *
 *------------------------------------------------------------------------------
 */

#include "rbtree.h"

// Non-interface functions, used internally
#ifndef RBTREE_USE_SLAB_ALLOC
static void _delete_tree( struct rbtree_node * );
#endif // RBTREE_USE_SLAB_ALLOC

static struct rbtree_node * _insert(
    struct rbtree *,
    struct rbtree_node *,
#ifdef RBTREE_TRACK_PARENT
    struct rbtree_node *,
#endif // RBTREE_TRACK_PARENT
    void *
);
static void _traverse_tree( struct rbtree_node *, void (*)(void *) );
static struct rbtree_node * _delete( struct rbtree *, struct rbtree_node *, void * );
static inline unsigned int _count_nodes( struct rbtree_node * );
static inline void color_flip( struct rbtree_node * );
static inline struct rbtree_node * rotate_left( struct rbtree_node * );
static inline struct rbtree_node * rotate_right( struct rbtree_node * );
#ifndef RBTREE_TEST
static inline struct rbtree_node * find_min( struct rbtree_node * );
#endif // RBTREE_TEST
static struct rbtree_node * del_min( struct rbtree *, struct rbtree_node * );
//static struct rbtree_node * find_max( struct rbtree_node * );
//static struct rbtree_node * del_max( struct rbtree *, struct rbtree_node * );
static inline struct rbtree_node * move_red_left( struct rbtree_node * );
static inline struct rbtree_node * move_red_right( struct rbtree_node * );
static inline struct rbtree_node * fix_up( struct rbtree_node * );

void rbtree_free_node(
#ifdef RBTREE_USE_SLAB_ALLOC
    struct rbtree * rb_tree,
#endif // RBTREE_USE_SLAB_ALLOC
    struct rbtree_node * node
)
{
#ifdef RBTREE_USE_SLAB_ALLOC
    struct rbtree_free_list * free_node = NULL;

    if( rb_tree->free_head == NULL )
    {
        free_node = ( struct rbtree_free_list * ) _RBTREE_ALLOC(
            sizeof( struct rbtree_free_list )
        );

        if( free_node == NULL )
        {
            return;
        }

        free_node->i = node->pool_i;
        free_node->j = node->pool_j;
        free_node->next = NULL;
        rb_tree->free_head = free_node;
    }
    else
    {
        free_node = rb_tree->free_head;

        while( free_node->next != NULL )
        {
            free_node = free_node->next;
        }

        free_node->next = ( struct rbtree_free_list * ) _RBTREE_ALLOC(
            sizeof( struct rbtree_free_list )
        );

        free_node = free_node->next;

        if( free_node == NULL )
        {
            return;
        }

        free_node->i = node->pool_i;
        free_node->j = node->pool_j;
        free_node->next = NULL;
    }

    rb_tree->_allocated--;
#ifdef RBTREE_DEBUG
    memset( node, 0, sizeof( struct rbtree_node ) );
#endif // RBTREE_DEBUG
#else
    _RBTREE_FREE( node );
#endif // RBTREE_USE_SLAB_ALLOC
    return;
}

struct rbtree_node * new_rbtree_node(
#ifdef RBTREE_USE_SLAB_ALLOC
    struct rbtree * rb_tree,
#endif // RBTREE_USE_SLAB_ALLOC
    void * data
)
{
    struct rbtree_node *      node      = NULL;
#ifdef RBTREE_USE_SLAB_ALLOC
    struct rbtree_free_list * free_node = NULL;
    unsigned int              i         = 0;
    unsigned int              j         = 0;

    if( rb_tree->_allocated < rb_tree->_total_size )
    {
        // There is free space in the pool
        if(
                rb_tree->pool_ind_j < rb_tree->pool_size
             && rb_tree->pool_ind_i < RBTREE_CHUNK_SIZE
          )
        {
            // We can grab an address pointed to free_i / free_j
            i = rb_tree->pool_ind_i;
            j = rb_tree->pool_ind_j;
            rb_tree->pool_ind_i++;

            if( rb_tree->pool_ind_i == RBTREE_CHUNK_SIZE )
            {
                rb_tree->pool_ind_i = 0;
                rb_tree->pool_ind_j++;
            }
        }
        else
        {
            // We'll need to scavenge memory
            free_node          = rb_tree->free_head;
            rb_tree->free_head = free_node->next;
            i                  = free_node->i;
            j                  = free_node->j;
            _RBTREE_FREE( free_node );
        }
    }
    else
    {
        rb_tree->pool = ( struct rbtree_node ** ) _RBTREE_REALLOC(
            rb_tree->pool,
            sizeof( struct rbtree_node * )
          * ( rb_tree->pool_size + 1 )
        );

        if( rb_tree->pool == NULL )
        {
            return NULL;
        }

        rb_tree->pool[rb_tree->pool_ind_j] = _RBTREE_ALLOC(
            sizeof( struct rbtree_node )
          * RBTREE_CHUNK_SIZE
        );

        rb_tree->pool_ind_i = 1;
        rb_tree->_total_size += RBTREE_CHUNK_SIZE;
        rb_tree->pool_size++;
        i = 0;
        j = rb_tree->pool_ind_j;
    }

    rb_tree->_allocated++;
    node = &(rb_tree->pool[j][i]);
#else
    node = ( struct rbtree_node * ) _RBTREE_ALLOC(
        sizeof( struct rbtree_node )
    );
#endif // RBTREE_USE_SLAB_ALLOC
    if( node == NULL || data == NULL )
    {
        if( node != NULL )
        {
            rbtree_free_node(
#ifdef RBTREE_USE_SLAB_ALLOC
                rb_tree,
#endif // RBTREE_USE_SLAB_ALLOC
                node
            );
        }
        return NULL;
    }

    memset( node, 0, sizeof( struct rbtree_node ) );
#ifdef RBTREE_USE_SLAB_ALLOC
    node->pool_i   = i;
    node->pool_j   = j;
#endif // RBTREE_USE_SLAB_ALLOC
    node->red      = true;
    node->data     = data;
    node->subcount = 1;
    return node;
}

struct rbtree * new_rbtree(
    bool (*compare)( void *, void * ),
    bool (*equal)( void *, void * )
)
{
    struct rbtree * rb_tree = NULL;

    rb_tree = ( struct rbtree * ) _RBTREE_ALLOC( sizeof( struct rbtree ) );

    if( rb_tree == NULL || compare == NULL )
    {
        if( rb_tree != NULL )
        {
            _RBTREE_FREE( rb_tree );
        }

        return NULL;
    }

    rb_tree->compare = compare;
    rb_tree->equal   = equal;
    rb_tree->rstack  = NULL;
    rb_tree->iter    = NULL;
    rb_tree->size    = 0;

#ifdef RBTREE_USE_SLAB_ALLOC
    rb_tree->pool = ( struct rbtree_node ** ) _RBTREE_ALLOC(
        sizeof( struct rbtree_node * )
    );

    if( rb_tree->pool == NULL )
    {
        _RBTREE_FREE( rb_tree );
        return NULL;
    }

    rb_tree->pool[0] = ( struct rbtree_node * ) _RBTREE_ALLOC(
        sizeof( struct rbtree_node )
      * RBTREE_CHUNK_SIZE
    );

    if( rb_tree->pool[0] == NULL )
    {
        _RBTREE_FREE( rb_tree->pool );
        _RBTREE_FREE( rb_tree );
        return NULL;
    }

    rb_tree->pool_size   = 1;
    rb_tree->pool_ind_i  = 0;
    rb_tree->pool_ind_j  = 0;
    rb_tree->_total_size = RBTREE_CHUNK_SIZE;
    rb_tree->_allocated  = 0;
#endif // RBTREE_USE_SLAB_ALLOC
    return rb_tree;
}

void free_rbtree( struct rbtree * rb_tree )
{
    rbtree_destroy( rb_tree );
    _RBTREE_FREE( rb_tree );
    return;
}

void rbtree_destroy( struct rbtree * rb_tree )
{
#ifdef RBTREE_USE_SLAB_ALLOC
    unsigned int              i         = 0;
    struct rbtree_free_list * free_node = NULL;
    struct rbtree_free_list * last      = NULL;

    for( i = 0; i < rb_tree->pool_size; i++ )
    {
        if( rb_tree->pool[i] != NULL )
        {
            _RBTREE_FREE( rb_tree->pool[i] );
        }
    }

    _RBTREE_FREE( rb_tree->pool );

    free_node = rb_tree->free_head;

    if( free_node != NULL )
    {
        while( free_node->next != NULL )
        {
            last = free_node;
            free_node = free_node->next;
            if( last != NULL )
            {
                _RBTREE_FREE( last );
            }
        }
    }
#else
    _delete_tree( rb_tree->root );
#endif // RBTREE_USE_SLAB_ALLOC
    rb_tree->root = NULL;
    return;
}

unsigned int rbtree_size( struct rbtree * rb_tree )
{
    rb_tree->size =_count_nodes( rb_tree->root );
    return rb_tree->size;
}

void rbtree_insert( struct rbtree * rb_tree, void * data )
{
    if( rb_tree == NULL || data == NULL )
    {
        return;
    }

    rb_tree->root = _insert(
        rb_tree,
        rb_tree->root,
#ifdef RBTREE_TRACK_PARENT
        NULL,
#endif // RBTREE_TRACK_PARENT
        data
    );

    rb_tree->root->red = false;
    rb_tree->size++;
    return;
}

void * rbtree_peek_position( struct rbtree * rb_tree, unsigned int n )
{
    struct rbtree_node * nth_node     = NULL;
    unsigned int         k            = 0;
    unsigned int         left_subtree = 0;

    if(
            n >= rb_tree->size
         || rb_tree->root == NULL
         || n >= rb_tree->root->subcount
      )
    {
        return NULL;
    }

    nth_node = rb_tree->root;
    k = n;

    while( nth_node != NULL )
    {
        left_subtree = nth_node->left == NULL ? 0 : nth_node->left->subcount;

        if( left_subtree == k )
        {
            return nth_node->data;
        }
        else
        {
            if( left_subtree < k )
            {
                k        = k - nth_node->left->subcount - 1;
                nth_node = nth_node->right;
            }
            else
            {
                nth_node = nth_node->left;
            }
        }
    }

    if( nth_node != NULL )
    {
        return nth_node->data;
    }

    return NULL;
}

unsigned int rbtree_get_position( struct rbtree * rb_tree, void * data )
{
    struct rbtree_node * node  = NULL;
    unsigned int         index = 0;

    node  = rb_tree->root;

    while( node != NULL )
    {
        if( rb_tree->compare( data, node->data ) )
        {
            node = node->left;
        }
        else
        {
            if(
                   data == node->data
                || (
                        rb_tree->equal != NULL
                     && rb_tree->equal( data, node->data )
                   )
              )
            {
                if( node->left != NULL )
                {
                    index += node->left->subcount;
                }

                return index;
            }
            else
            {
                if( node->left != NULL )
                {
                    index += node->left->subcount + 1;
                }

                node = node->right;
            }
        }
    }

    return 0;

}

struct rbtree_node * rbtree_search( struct rbtree * rb_tree, void * data )
{
    struct rbtree_node * node = NULL;

    if( rb_tree == NULL )
    {
        return NULL;
    }

    node = rb_tree->root;

    while( node != NULL )
    {
        if( rb_tree->compare( data, node->data ) )
        {
            node = node->right;
        }
        else if(
                    data == node->data
                 || ( rb_tree->equal != NULL && rb_tree->equal( data, node->data ) )
               )
        {
            return node;
        }
        else
        {
            node = node->left;
        }
    }

    return NULL;
}

void rbtree_delete( struct rbtree * rb_tree, void * data )
{
    if( data == NULL )
    {
        return;
    }

    rb_tree->root = _delete( rb_tree, rb_tree->root, data );

    if( rb_tree->root != NULL )
    {
        rb_tree->root->red = false;
    }

    rb_tree->size--;

    return;
}

void rbtree_foreach(
    struct rbtree * rb_tree,
    void (*f)(void *)
)
{
    if( f == NULL )
    {
        return;
    }

    return _traverse_tree( rb_tree->root, f );
}

void * rbtree_pop( struct rbtree * rb_tree )
{
    struct rbtree_node * node = NULL;
    void * data               = NULL;

    node = find_min( rb_tree->root );
    data = node->data;

    rb_tree->root = del_min( rb_tree, rb_tree->root );
    rb_tree->size--;
    return data;
}

void rbtree_iter_begin( struct rbtree * rb_tree )
{
    rb_tree->rstack = NULL;
    rb_tree->iter   = rb_tree->root;
    return;
}

void rbtree_iter_reset( struct rbtree * rb_tree )
{
    rb_tree->rstack = NULL;
    rb_tree->iter   = NULL;
    return;
}

struct rbtree_node * rbtree_iter_next( struct rbtree * rb_tree )
{
    struct rbtree_node * node = NULL;

    while( rb_tree->rstack != NULL || rb_tree->iter != NULL )
    {
        if( rb_tree->iter != NULL )
        {
            rbtree_stack_push( rb_tree->rstack, rb_tree->iter );
            rb_tree->iter = rb_tree->iter->left;
        }
        else
        {
            rb_tree->iter = rbtree_stack_top( rb_tree->rstack );
            rbtree_stack_pop( rb_tree->rstack );
            node = rb_tree->iter;
            rb_tree->iter = rb_tree->iter->right;
            break;
        }
    }

    return node;
}

void rbtree_delete_min( struct rbtree * rb_tree )
{
    if( rb_tree->size == 0 )
    {
        return;
    }

    rb_tree->root = del_min( rb_tree, rb_tree->root );
    rb_tree->size--;
    return;
}

static inline unsigned int _count_nodes( struct rbtree_node * node )
{
    if( node == NULL )
    {
        return 0;
    }

    return node->subcount;
}

static struct rbtree_node * _insert(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree,
#ifdef RBTREE_TRACK_PARENT
    struct rbtree_node * last,
#endif //RBTREE_TRACK_PARENT
    void *               data
)
{
    struct rbtree_node * node = NULL;

    if( tree == NULL )
    {
        node = new_rbtree_node(
#ifdef RBTREE_USE_SLAB_ALLOC
            rb_tree,
#endif // RBTREE_USE_SLAB_ALLOC
            data
        );

#ifdef RBTREE_TRACK_PARENT
        if( last != NULL )
        {
            node->parent = last;
        }
#endif // RBTREE_TRACK_PARENT

        return node;
    }

    if( rb_tree->compare( data, tree->data ) )
    {
        tree->subcount++;
        tree->left = _insert(
            rb_tree,
            tree->left,
#ifdef RBTREE_TRACK_PARENT
            tree,
#endif // RBTREE_TRACK_PARENT
            data
        );
    }
    else
    {
        if(
               data == tree->data
            || (
                   rb_tree->equal != NULL
                && rb_tree->equal( data, tree->data )
               )
          )
        {
            return tree;
        }

        tree->subcount++;
        tree->right = _insert(
            rb_tree,
            tree->right,
#ifdef RBTREE_TRACK_PARENT
            tree,
#endif // RBTREE_TRACK_PARENT
            data
        );
    }

#ifdef RBTREE_TRACK_PARENT
    if( last != NULL )
    {
        tree->parent = last;
    }
#endif // RBTREE_TRACK_PARENT

    if( is_red( tree->right ) )
    {
        tree = rotate_left( tree );
    }

    if( is_red( tree->left ) && is_red( tree->left->left ) )
    {
        tree = rotate_right( tree );
    }

    if( is_red( tree->left ) && is_red( tree->right ) )
    {
        color_flip( tree );
    }

    return tree;
}

#ifndef RBTREE_USE_SLAB_ALLOC
static void _delete_tree( struct rbtree_node * node )
{
    if( node == NULL )
    {
        return;
    }

    _delete_tree( node->left );
    _delete_tree( node->right );

    _RBTREE_FREE(node);
    return;
}
#endif // RBTREE_USE_SLAB_ALLOC

static struct rbtree_node * _delete(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree,
    void *               data
)
{
    struct rbtree_node * r_min = NULL;
    bool                 equal = false;

    if( tree == NULL )
    {
        return NULL;
    }

    equal = (
                data == tree->data
             || (
                    rb_tree->equal != NULL
                 && rb_tree->equal( data, tree->data )
                )
            );

    if( rb_tree->compare( data, tree->data ) )
    {
        if(
               !is_red( tree->left )
            && tree->left != NULL
            && !is_red( tree->left->left )
          )
        {
            tree = move_red_left( tree );
        }

        tree->subcount--;
        tree->left = _delete( rb_tree, tree->left, data );
    }
    else
    {
        if( is_red( tree->left ) )
        {
            tree = rotate_right( tree );
        }

        if( equal && ( tree->right == NULL ) )
        {
            rbtree_free_node(
#ifdef RBTREE_USE_SLAB_ALLOC
                rb_tree,
#endif // RBTREE_USE_SLAB_ALLOC
                tree
            );
            return NULL;
        }

        if(
                !is_red( tree->right )
             && tree->right != NULL
             && !is_red( tree->right->left )
          )
        {
            tree = move_red_left( tree );
        }

        if(
                data == tree->data
             || (
                    rb_tree->equal != NULL
                 && rb_tree->equal( data, tree->data )
                )
          )
        {
            r_min = find_min( tree->right );
            tree->data = r_min->data;
            tree->subcount--;
            tree->right = del_min( rb_tree, tree->right );
        }
        else
        {
            tree->subcount--;
            tree->right = _delete( rb_tree, tree->right, data );
        }
    }

    return fix_up( tree );
}

static void _traverse_tree(
    struct rbtree_node * node,
    void (*f)(void *)
)
{
    if( node == NULL )
    {
        return;
    }

    _traverse_tree( node->left, f );
#ifdef RBTREE_DEBUG
#ifdef RBTREE_TRACK_PARENT
#define RBTREE_NODE_OUT_STR "Node: %p, L %p, P %p, R %p, dat %p, SC %u"
#else
#define RBTREE_NODE_OUT_STR "Node: %p, L %p, R %p, dat %p, SC %u"
#endif
    _RBTREE_LOG(
        RBTREE_NODE_OUT_STR,
        node,
        node->left,
#ifdef RBTREE_TRACK_PARENT
        node->parent,
#endif // RBTREE_TRACK_PARENT
        node->right,
        node->data,
        node->subcount
    );
#endif // RBTREE_DEBUG
    f( node->data );
    _traverse_tree( node->right, f );
    return;
}

static inline void color_flip( struct rbtree_node * tree )
{
    tree->red        = !tree->red;
    tree->left->red  = !tree->left->red;
    tree->right->red = !tree->right->red;

    return;
}

/*
 *  Transform tree from
 *    A          B
 *     \        /
 *      B  to  A
 *     /        \
 *    x          x
 */
static inline struct rbtree_node * rotate_left( struct rbtree_node * a )
{
    struct rbtree_node * b = NULL;
#ifdef RBTREE_TRACK_PARENT
    struct rbtree_node * c = NULL; //parent

    c           = a->parent;
#endif // RBTREE_TRACK_PARENT
    b           = a->right;
    a->right    = b->left;
    b->left     = a;
#ifdef RBTREE_TRACK_PARENT
    a->parent   = b;
    b->parent   = c;
#endif // RBTREE_TRACK_PARENT
    b->red      = a->red;
    a->red      = true;
    a->subcount = 1
                + ( a->right == NULL ? 0 : a->right->subcount )
                + ( a->left  == NULL ? 0 : a->left->subcount );
    b->subcount = 1
                + a->subcount
                + ( b->right == NULL ? 0 : b->right->subcount );

#ifdef RBTREE_TRACK_PARENT
    if( a->right != NULL )
    {
        a->right->parent = a;
    }
#endif // RBTREE_TRACK_PARENT
    return b;
}

/*
 *  Transform tree from
 *     A         B
 *    /           \
 *   B     to      A
 *    \           /
 *     x         x
 */
static inline struct rbtree_node * rotate_right( struct rbtree_node * a )
{
    struct rbtree_node * b = NULL;
#ifdef RBTREE_TRACK_PARENT
    struct rbtree_node * c = NULL; // parent

    c           = a->parent;
#endif // RBTREE_TRACK_PARENT
    b           = a->left;
    a->left     = b->right;
    b->right    = a;
#ifdef RBTREE_TRACK_PARENT
    b->parent   = c;
    a->parent   = b;
#endif // RBTREE_TRACK_PARENT
    b->red      = a->red;
    a->red      = true;
    a->subcount = 1
                + ( a->left  == NULL ? 0 : a->left->subcount )
                + ( a->right == NULL ? 0 : a->right->subcount );
    b->subcount = 1
                + a->subcount
                + ( b->left  == NULL ? 0 : b->left->subcount );

#ifdef RBTREE_TRACK_PARENT
    if( a->left != NULL )
    {
        a->left->parent = a;
    }
#endif // RBTREE_TRACK_PARENT
    return b;
}

#ifndef RBTREE_TEST
static inline
#endif // RBTREE_TEST
struct rbtree_node * find_min( struct rbtree_node * tree )
{
    if( tree == NULL )
    {
        return NULL;
    }

    while( tree->left )
    {
        tree = tree->left;
    }

    return tree;
}

static inline struct rbtree_node * del_min(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree
)
{
    if( tree->left == NULL )
    {
        rbtree_free_node(
#ifdef RBTREE_USE_SLAB_ALLOC
            rb_tree,
#endif // RBTREE_USE_SLAB_ALLOC
            tree
        );
        return NULL;
    }

    if(
            !is_red( tree->left )
         && !is_red( tree->left->left )
      )
    {
        tree = move_red_left( tree );
    }

    tree->subcount--;
    tree->left = del_min( rb_tree, tree->left );

    return fix_up( tree );
}

/* unused, untested
static struct rbtree_node * find_max( struct rbtree_node * tree )
{
    if( tree == NULL )
    {
        return NULL;
    }

    while( tree->right )
    {
        tree = tree->right;
    }

    return tree;
}

static struct rbtree_node * del_max(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree
)
{
    if( tree->right == NULL )
    {
#ifdef RBTREE_TRACK_PARENT
        if( tree->parent != NULL )
        {
            tree->parent->right = NULL;
        }
#endif // RBTREE_TRACK_PARENT
        rb_tree->size--;
        _RBTREE_FREE( tree );
        return NULL;
    }

    if(
            !is_red( tree->right )
         && !is_red( tree->right->right )
      )
    {
        tree = move_red_right( tree );
    }

    tree->subcount--;
    tree->right = del_max( rb_tree, tree->right );
    return fix_up( tree );
}
*/
static inline struct rbtree_node * move_red_left( struct rbtree_node * tree )
{
    color_flip( tree );

    if( is_red( tree->right->left ) )
    {
        tree->right = rotate_right( tree->right );
        tree        = rotate_left( tree );
        color_flip( tree );
    }

    return tree;
}

static inline struct rbtree_node * move_red_right( struct rbtree_node * tree )
{
    color_flip( tree );

    if( is_red( tree->left->left ) )
    {
        tree = rotate_right( tree );
        color_flip( tree );
    }

    return tree;
}

static inline struct rbtree_node * fix_up( struct rbtree_node * tree )
{
    if( is_red( tree->right ) )
    {
        tree = rotate_left( tree );
    }

    if( is_red( tree->left ) && is_red( tree->left->left ) )
    {
        tree = rotate_right( tree );
    }

    if( is_red( tree->left ) && is_red( tree->right ) )
    {
        color_flip( tree );
    }

    return tree;
}

#ifdef RBTREE_DEBUG
void rbtree_setup_debug( struct rbtree * rb_tree, void (*debug)( void * ) )
{
    rb_tree->debug = debug;
    return;
}

void rbtree_setup_pretty_print( struct rbtree * rb_tree, char *(*pretty)( void * ) )
{
    rb_tree->pretty_print = pretty;
}

void rbtree_dummy_debug( void * data )
{
    return;
}

// XXX todo
#define TEE "|-"
#define END "--"
#define INDENT "| "

void rbtree_debug( struct rbtree * rb_tree )
{
    if( rb_tree->pretty_print != NULL )
    {

        return;
    }
    else
    {
        _RBTREE_LOG(
            "Dumping rbtree %p, root %p, size %u\n",
            rb_tree,
            rb_tree->root,
            rb_tree->size
        );
        rbtree_foreach( rb_tree, rb_tree->debug );
    }

    return;
}
#endif

#ifdef RBTREE_TEST
void rbtree_log( char * msg, ... )
{
    va_list args = {{0}};

    if( msg == NULL )
    {
        return;
    }

    va_start( args, msg );

    vfprintf( stdout, msg, args );
    fprintf( stdout, "\n" );
    va_end( args );
    fflush( stdout );
    return;
}
#endif // RBTREE_TEST
