#include "rbtree_test.h"
/*
 * For this test we will insert a boatload of data and validate the tree after manipulation
 */

bool compare( void * a, void * b )
{
    if( a == NULL || b == NULL )
    {
        _ERROR( "RBtree test compare function got NULL input" );
    }

    return *( ( unsigned int * ) a ) < *( ( unsigned int * ) b );
}

bool equal( void * a, void * b )
{
    if( a == NULL || b == NULL )
    {
        _ERROR( "RBTree test equality function got NULL input" );
    }

    return *( unsigned int * )a == *( unsigned int * )b;
}

bool check_tree_health( struct rbtree * rb_tree )
{
    if( rb_tree == NULL )
    {
        return true;
    }

    if(
            ( rb_tree->size == 0 && rb_tree->root != NULL )
         || ( rb_tree->size > 0 && rb_tree->root == NULL )
      )
    {
        printf( "Health check failed, tree is empty and root is set or root is empty and tree has nodes\n" );
        return false;
    }

    if( rb_tree->size == 0 && rb_tree->root == NULL )
    {
        return true;
    }

    if( rb_tree->size != rb_tree->root->subcount )
    {
        printf( "Health check failed, root subcount and tree size mismatch\n" );
        return false;
    }

    return __check_tree_health( rb_tree->root );
}

bool __check_tree_health( struct rbtree_node * tree )
{
    bool result = false;
    unsigned int sc_check = 0;

    if( tree == NULL )
    {
        return true;
    }

    sc_check = 1;

    if( tree->left != NULL )
    {
        sc_check += tree->left->subcount;
        result = __check_tree_health( tree->left );
    }

    if( tree->right != NULL )
    {
        sc_check += tree->right->subcount;
        result = __check_tree_health( tree->right );
    }

    if( tree->right == NULL && tree->left == NULL )
    {
        result = true;
    }
#ifdef RBTREE_TRACK_PARENT
    if( tree->parent != NULL )
    {
        if( tree->parent->left != tree && tree->parent->right != tree )
        {
            _ERROR( "\nRBTREE health check failed, detached parent\n" );
            result = false;
        }
    }
#endif // RBTREE_TRACK_PARENT
    if( sc_check != tree->subcount )
    {
        printf( "Expected subcount %u, got %u at node %p\n", sc_check, tree->subcount, tree );
        _ERROR( "\nRBTREE health check failed, subcount incorrect\n" );
    }

    return result;
}

void __rbtree_test( void )
{
    struct rbtree_index ** check_data = NULL;
    struct rbtree_node *   node       = NULL;
    struct rbtree *        rb_tree    = NULL;
    unsigned int **        test_data  = NULL;
    void *                 data       = NULL;
    void *                 data_2     = NULL;
    unsigned int           i          = 0;
    unsigned int           k          = 0;
    unsigned int           j          = 0;
    unsigned int *         arr        = NULL;
    bool                   match      = false;

    srandom( (unsigned int) time( NULL ) );
    rb_tree = new_rbtree( &compare, &equal );
    rbtree_setup_debug( rb_tree, &rbtree_dummy_debug );
    __allocate( &check_data, &test_data );

    if( rb_tree == NULL )
    {
        _ERROR( "new_rbtree() returned NULL result" );
    }

    __rbtree_insert_data( rb_tree, test_data );
    __setup_check_data( rb_tree, test_data, check_data );
    printf( "Checking RBTree health..." );

    if( !check_tree_health( rb_tree ) )
    {
        _ERROR( "Failed\n" );
    }

    printf( " Done.\nChecking RBTree iterand..." );
    rbtree_iter_begin( rb_tree );

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        node = rbtree_iter_next( rb_tree );
        if( node == NULL )
        {
            _ERROR(
                "\nRBTREE iter gave NULL for in-bounds access\n"
            );
        }

        if( *(( unsigned int * ) node->data) != *( (unsigned int * )check_data[i]->data) )
        {
            _ERROR(
                "\nRBTREE iter index mismatched with reference data set\n"
            );
        }
    }

    printf( " Done.\nChecking RBTree position peek..." );

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        data = rbtree_peek_position( rb_tree, i );

        if( data == NULL )
        {
            _ERROR( "\nRBTREE position peek gave NULL for in-bounds access\n" );
        }

        if( *((unsigned int *)data) != *((unsigned int *)check_data[i]->data) )
        {
            _ERROR( "\nRBTREE position peek index mismatched with reference data set\n" );
        }
    }

    printf( " Done.\nChecking get position..." );
    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        k = rbtree_get_position( rb_tree, check_data[i]->data );

        if( k != i )
        {
            printf( "Expecting index %u, got %u\n", i, k );
            _ERROR( "\nRBTREE get position returned incorrect index\n" );
        }
    }

    printf( " Done.\nChecking Find Min / Delete Min..." );
    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        node = find_min( rb_tree->root );

        if( node == NULL || node->data == NULL )
        {
            _ERROR( "\nRBTREE find min gave NULL result\n" );
        }

        if( *((unsigned int * )node->data) != *((unsigned int *)check_data[i]->data ) )
        {
            _ERROR( "\nRBTREE min result mismatch\n" );
        }

        //Add delete
        rbtree_delete_min( rb_tree );
        node = find_min( rb_tree->root );

        if( node == NULL && i == ( TEST_NODE_COUNT - 1 ) )
        {
            continue;
        }

        if( node == NULL || node->data == NULL )
        {
            _ERROR( "\nRBTREE  find_min returned NULL following delete\n" );
        }

        if( *((unsigned int *)node->data ) == *((unsigned int *)check_data[i]->data ) )
        {
            _ERROR( "\nRBTREE find_min returned incorrect result following delete\n" );
        }

        if( !check_tree_health( rb_tree ) )
        {
            _ERROR( "\nRBTREE failed health check following delete\n" );
        }
    }

    // Repopulate the tree so that we can test the node delete functionality
    __deallocate( &check_data, &test_data );
    free( rb_tree );
    rb_tree = new_rbtree( &compare, &equal );
    rbtree_setup_debug( rb_tree, &rbtree_dummy_debug );
    __allocate( &check_data, &test_data );
    __rbtree_insert_data( rb_tree, test_data );
    __setup_check_data( rb_tree, test_data, check_data );

    printf( " Done.\nChecking find nth / delete..." );

    for( i = TEST_NODE_COUNT - 1; i > 0; i-- )
    {
        data = rbtree_peek_position( rb_tree, i );
        if( data == NULL )
        {
            _ERROR( "\nRBTREE nth node failed with NULL result\n" );
        }

        rbtree_delete( rb_tree, data );

        if( i > 0 )
        {
            if( !check_tree_health( rb_tree ) )
            {
                _ERROR( "\nRBTREE health check failed following node deletion\n" );
            }

            data = rbtree_peek_position( rb_tree, i );

            if( data != NULL )
            {
                _ERROR( "\nRBTREE returned non NULL result for out of bounds access\n" );
            }

            data_2 = rbtree_peek_position( rb_tree, i - 1 );

            if( data_2 != NULL &&  data_2 == data )
            {
                _ERROR( "\nRBTree returned incorrect data following deletion\n" );
            }
        }
        else
        {
            if( rb_tree->root != NULL || rb_tree->size != 0 )
            {
                _ERROR( "\nRBTree should be empty\n" );
            }
        }
    }

    printf( " Done.\nChecking Random delete..." );

    __deallocate( &check_data, &test_data );
    free( rb_tree );
    rb_tree = new_rbtree( &compare, &equal );
    rbtree_setup_debug( rb_tree, &rbtree_dummy_debug );
    __allocate( &check_data, &test_data );
    __rbtree_insert_data( rb_tree, test_data );
    __setup_check_data( rb_tree, test_data, check_data );

    arr = ( unsigned int * ) calloc( TEST_NODE_COUNT, sizeof( unsigned int ) );
    if( arr == NULL )
    {
        _ERROR( "Failed to allocate array for random node access\n" );
    }

    j = 0;

    while( j < TEST_NODE_COUNT )
    {
        k = ( random() % TEST_NODE_COUNT );

        match = false;
        for( i = 0; i < j; i++ )
        {
            if( arr[i] == k )
            {
                match = true;
            }
        }

        if( match == false )
        {
            arr[j] = k;
            j++;
        }
    }

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        j = arr[i];

        if( j >= rb_tree->size )
        {
            continue;
        }

        data = rbtree_peek_position( rb_tree, j );
        k    = rb_tree->size;
        if( data == NULL )
        {
            _ERROR( "\nRBTREE random access returned NULL\n" );
        }

        rbtree_delete( rb_tree, data );

        if( k - 1 != rb_tree->size )
        {
            _ERROR( "\nRBTREE size did not decrement\n" );
        }

        check_tree_health( rb_tree );
    }

    __deallocate( &check_data, &test_data );
    free( arr );
    printf( " Done\n" );
    return;
}

void __rbtree_insert_data( struct rbtree * rb_tree, unsigned int ** data )
{
    unsigned int   i    = 0;

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        *(data[i]) = (unsigned int) random();
        rbtree_insert( rb_tree, ( void * ) data[i] );
    }

    return;
}

// Don't judge me, this implements a naive sort that's probably O(n^n^n)
void __setup_check_data( struct rbtree * rb_tree, unsigned int ** test_data, struct rbtree_index ** sorted_test )
{
    struct rbtree_index   temp        = {0};
    struct rbtree_node *  node        = NULL;
    unsigned int          i           = 0;
    unsigned int          k           = 0;

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        sorted_test[i]->data = ( void * ) test_data[i];
        sorted_test[i]->node = rbtree_search( rb_tree, ( void * ) test_data[i] );
    }

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        for( k = i + 1; k < TEST_NODE_COUNT; k++ )
        {
            if(
                    compare(
                        sorted_test[k]->data,
                        sorted_test[i]->data
                    )
              )
            { // k < i
                temp.data = sorted_test[i]->data;
                temp.node = sorted_test[i]->node;
                sorted_test[i]->data = sorted_test[k]->data;
                sorted_test[i]->node = sorted_test[k]->node;
                sorted_test[k]->data = temp.data;
                sorted_test[k]->node = temp.node;
            }
        }
    }

    return;
}

void __allocate( struct rbtree_index *** check_data, unsigned int *** data )
{
    unsigned int           i            = 0;
    struct rbtree_index ** t_check_data = NULL;
    unsigned int **        t_data       = NULL;

    t_check_data = ( struct rbtree_index ** ) calloc(
        TEST_NODE_COUNT,
        sizeof( struct rbtree_index * )
    );

    t_data = ( unsigned int ** ) calloc(
        TEST_NODE_COUNT,
        sizeof( unsigned int * )
    );

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        t_data[i] = ( unsigned int * ) calloc(
            1,
            sizeof( unsigned int )
        );
        t_check_data[i] = ( struct rbtree_index * ) calloc(
            1,
            sizeof( struct rbtree_index )
        );
    }

    (*data) = t_data;
    (*check_data) = t_check_data;
    return;
}

void __deallocate( struct rbtree_index *** check_data, unsigned int *** data )
{
    unsigned int           i            = 0;
    struct rbtree_index ** t_check_data = NULL;
    unsigned int **        t_data       = NULL;

    t_data = (*data);
    t_check_data = (*check_data);

    for( i = 0; i < TEST_NODE_COUNT; i++ )
    {
        free( t_data[i] );
        free( t_check_data[i] );
        t_data[i] = NULL;
        t_check_data[i] = NULL;
    }

    free( t_data );
    free( t_check_data );
    (*data) = NULL;
    (*check_data) = NULL;
    return;
}
