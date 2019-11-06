#include "rbtree.h"
#include "test_common.h"

#define TEST_NODE_COUNT 32

// We will have an array[n] of these, in sorted order (by the uint pointer)
// The other pointer will index to the rbtree_node. This also gives us the ability
// to validate rbtree_foreach / tree traversal
struct rbtree_index {
    unsigned int * data;
    struct rbtree_node * node;
};


void __rbtree_test( void );
void __rbtree_insert_data( struct rbtree *, unsigned int **);
void __setup_check_data( struct rbtree *, unsigned int **, struct rbtree_index ** );
void __allocate( struct rbtree_index ***, unsigned int *** );
void __deallocate( struct rbtree_index ***, unsigned int *** );
bool __check_tree_health( struct rbtree_node * );
