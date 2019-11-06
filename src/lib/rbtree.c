#include "rbtree.h"

struct rbtree_node * new_rbtree_node( void * data )
{
    struct rbtree_node * node = NULL;

    if( data == NULL )
    {
        return NULL;
    }

    node = ( struct rbtree_node * ) _RBTREE_ALLOC(
        sizeof( struct rbtree_node )
    );

    if( node == NULL )
    {
        return NULL;
    }

    node->red      = true;
    node->data     = data;
    node->left     = NULL;
    node->right    = NULL;
    node->parent   = NULL;
    node->subcount = 1;
    return node;
}

struct rbtree * new_rbtree(
    bool (*compare)( void *, void * ),
    bool (*equal)(void *, void * )
)
{
    struct rbtree * rb_tree = NULL;

    if( compare == NULL || equal == NULL )
    {
        return NULL;
    }

    rb_tree = ( struct rbtree * ) _RBTREE_ALLOC( sizeof( struct rbtree ) );

    if( rb_tree == NULL )
    {
        return NULL;
    }

    rb_tree->compare = compare;
    rb_tree->equal   = equal;
    rb_tree->rstack  = NULL;
    rb_tree->iter    = NULL;
    rb_tree->size    = 0;

    return rb_tree;
}

void free_rbtree( struct rbtree * rb_tree )
{
    rbtree_destroy( rb_tree );
    _RBTREE_FREE( rb_tree );
    return;
}

void _delete_tree( struct rbtree_node * node )
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

void rbtree_destroy( struct rbtree * rb_tree )
{
    _delete_tree( rb_tree->root );
    rb_tree->root = NULL;
    return;
}

unsigned int _count_nodes( struct rbtree_node * node )
{
    if( node == NULL )
    {
        return 0;
    }

    return node->subcount;
}

unsigned int rbtree_size( struct rbtree * rb_tree )
{
    rb_tree->size =_count_nodes( rb_tree->root );
    return rb_tree->size;
}

struct rbtree_node * _insert(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree,
    struct rbtree_node * last,
    void *               data
)
{
    struct rbtree_node * node = NULL;

    if( tree == NULL )
    {
        node = new_rbtree_node( data );

        if( last != NULL )
        {
            node->parent = last;
        }

        return node;
    }

    if( rb_tree->equal( data, tree->data ) )
    {
        return tree;
    }
    else if( rb_tree->compare( data, tree->data ) )
    {
        tree->subcount++;
        tree->left = _insert( rb_tree, tree->left, tree, data );
    }
    else
    {
        tree->subcount++;
        tree->right = _insert( rb_tree, tree->right, tree, data );
    }

    if( last != NULL )
    {
        tree->parent = last;
    }

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

void rbtree_insert( struct rbtree * rb_tree, void * data )
{
    if( rb_tree == NULL || data == NULL )
    {
        return;
    }

    rb_tree->root      = _insert( rb_tree, rb_tree->root, NULL, data );
    rb_tree->root->red = false;
    rb_tree->size++;
    return;
}

struct rbtree_node * _nth_node(
    struct rbtree_node * node,
    unsigned int         n
)
{
    struct rbtree_node * temp         = NULL;
    unsigned int         k            = 0;
    unsigned int         left_subtree = 0;

    temp = node;
    k    = n;

    while( temp != NULL )
    {
        left_subtree = temp->left == NULL ? 0 : temp->left->subcount;

        if( left_subtree == k )
        {
            return temp;
        }
        else if( left_subtree < k )
        {
            k    = k - temp->left->subcount - 1;
            temp = temp->right;

        }
        else
        {
            temp = temp->left;
        }
    }

    return NULL;
}

struct rbtree_node * rbtree_nth_node( struct rbtree * rb_tree, unsigned int n )
{
    struct rbtree_node * node = NULL;

    if( rb_tree->root == NULL || n >= rb_tree->root->subcount )
    {
        return NULL;
    }

    node = _nth_node( rb_tree->root, n );
    return node;
}

void * rbtree_peek_position( struct rbtree * rb_tree, unsigned int n )
{
    struct rbtree_node * nth_node = NULL;

    if( n >= rb_tree->size )
    {
        return NULL;
    }

    nth_node = rbtree_nth_node( rb_tree, n );

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
            if( rb_tree->equal( data, node->data ) )
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

struct rbtree_node * _delete(
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

    equal = rb_tree->equal( data, tree->data );

    if( rb_tree->compare( data, tree->data ) )
    {
        if(
               !is_red( tree->left )
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
            _RBTREE_FREE( tree );
            return NULL;
        }

        if(
                !is_red( tree->right )
             && !is_red( tree->right->left )
          )
        {
            tree = move_red_left( tree );
        }

        if( rb_tree->equal( data, tree->data ) )
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
        else if( rb_tree->equal( data, node->data ) )
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

void _traverse_tree(
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
    _RBTREE_LOG(
        "Node: %p L %p, P %p, R %p dat %p SC %u",
        node,
        node->left,
        node->parent,
        node->right,
        node->data,
        node->subcount
    );
#endif // RBTREE_DEBUG
    f( node->data );
    _traverse_tree( node->right, f );
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

void color_flip( struct rbtree_node * tree )
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
struct rbtree_node * rotate_left( struct rbtree_node * a )
{
    struct rbtree_node * b = NULL;
    struct rbtree_node * c = NULL; //parent

    c           = a->parent;
    b           = a->right;
    a->right    = b->left;
    b->left     = a;
    a->parent   = b;
    b->parent   = c;
    b->red      = a->red;
    a->red      = true;
    a->subcount = 1
                + ( a->right == NULL ? 0 : a->right->subcount )
                + ( a->left  == NULL ? 0 : a->left->subcount );
    b->subcount = 1
                + a->subcount
                + ( b->right == NULL ? 0 : b->right->subcount );

    if( a->right != NULL )
    {
        a->right->parent = a;
    }

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
struct rbtree_node * rotate_right( struct rbtree_node * a )
{
    struct rbtree_node * b = NULL;
    struct rbtree_node * c = NULL; // parent

    c           = a->parent;
    b           = a->left;
    a->left     = b->right;
    b->right    = a;
    b->parent   = c;
    a->parent   = b;
    b->red      = a->red;
    a->red      = true;
    a->subcount = 1
                + ( a->left  == NULL ? 0 : a->left->subcount )
                + ( a->right == NULL ? 0 : a->right->subcount );
    b->subcount = 1
                + a->subcount
                + ( b->left  == NULL ? 0 : b->left->subcount );

    if( a->left != NULL )
    {
        a->left->parent = a;
    }

    return b;
}

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

struct rbtree_node * del_min(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree
)
{
    if( tree->left == NULL )
    {
        _RBTREE_FREE( tree );
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

struct rbtree_node * find_max( struct rbtree_node * tree )
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

struct rbtree_node * del_max(
    struct rbtree *      rb_tree,
    struct rbtree_node * tree
)
{
    if( tree->right == NULL )
    {
        if( tree->parent != NULL )
        {
            tree->parent->right = NULL;
        }

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

struct rbtree_node * move_red_left( struct rbtree_node * tree )
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

struct rbtree_node * move_red_right( struct rbtree_node * tree )
{
    color_flip( tree );

    if( is_red( tree->left->left ) )
    {
        tree = rotate_right( tree );
        color_flip( tree );
    }

    return tree;
}

struct rbtree_node * fix_up( struct rbtree_node * tree )
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
