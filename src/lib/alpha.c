/*------------------------------------------------------------------------------
 * alpha.c
 *     Alpha shape computation library
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      alpha.c
 *
 *------------------------------------------------------------------------------
 */

#include "alpha.h"

bool get_alpha_shape(
    Point **       point_array,
    unsigned int   point_array_length,
    double         alpha,
    POLYGON **     alpha_shape,
    unsigned int * alpha_shape_length
)
{
    LSEG **      edge_list        = NULL;
    unsigned int i                = 0;
    unsigned int j                = 0;
    unsigned int k                = 0;
    unsigned int num_combinations = 0;

    if( point_array == NULL || point_array_length < 4 )
    {
        return false;
    }

    // Edge list will be nCr long:
    //  C( point_array_length, 2 )
    num_combinations = ( unsigned int ) combinations( point_array_length, 2 );

    edge_list = ( LSEG ** ) palloc0(
        sizeof( LSEG ** )
      * num_combinations
    );

    if( edge_list == NULL )
    {
        return false;
    }

    for( i = 0; i < num_combinations - point_array_length + 1; i++ )
    {
        for( j = i + 1; j < num_combinations - point_array_length + 2; j++ )
        {
            edge_list[k] = ( LSEG * ) palloc( sizeof( LSEG ) );

            if( edge_list[k] == NULL )
            {
                for( i = 0; i < k; i++ )
                {
                    pfree( edge_list[k] );
                }

                pfree( edge_list );
                return false;
            }

            edge_list[k]->p[0].x = point_array[i]->x;
            edge_list[k]->p[0].y = point_array[i]->y;
            edge_list[k]->p[1].x = point_array[j]->x;
            edge_list[k]->p[1].y = point_array[j]->y;
            k++;
        }
    }

    return true;
}

inline unsigned long int combinations( unsigned long int n, unsigned long int r )
{
    /*
     * C(n,r) = n! / ( r! * ( n - r )! )
     */
    return ( unsigned long int ) factorial( n )
         / ( factorial( r ) * factorial( n - r ) );
}

inline unsigned long int factorial( unsigned long int n )
{
    register unsigned long int fact = 2;
    register unsigned long int i    = 0;

    if( n <= 2 )
    {
        if( n == 0 )
        {
            return 1;
        }

        return n;
    }

    for( i = 3; i <= n; i++ )
    {
        fact *= i;
    }

    return fact;
}
