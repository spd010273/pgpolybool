/*------------------------------------------------------------------------------
 * box_funcs.c
 *      Helper functions of pgpolybool box functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      box_funcs.c
 *
 *------------------------------------------------------------------------------
 */

#include "box_funcs.h"

Point ** get_box_points( BOX * box )
{
    unsigned int i            = 0;
    double       Um           = 0.0;
    double       Ub           = 0.0;
    double       L            = 0.0;
    double       a            = 0.0;
    double       b            = 0.0;
    double       c            = 0.0;
    Point        center       = {0};
    Point **     result_array = NULL;

    if( box == NULL )
    {
        return NULL;
    }

    // Allocate space for our array and its elements
    result_array = ( Point ** ) palloc0( sizeof( Point * ) * 4 );

    if( result_array == NULL )
    {
        return NULL;
    }

    for( i = 0; i < 4; i++ )
    {
        result_array[i] = ( Point * ) palloc0( sizeof( Point ) );

        if( result_array[i] == NULL )
        {
            for( i = 0; i < 4; i++ )
            {
                if( result_array[i] != NULL )
                {
                    pfree( result_array[i] );
                }
            }

            pfree( result_array );
            return NULL;
        }
    }

    // Fill in the points we have
    result_array[0]->x = box->high.x; // Point A
    result_array[0]->y = box->high.y;
    result_array[2]->x = box->low.x; // Point C
    result_array[2]->y = box->low.y;

    center.x = ( box->high.x + box->low.x ) / 2.0;
    center.y = ( box->high.y + box->low.y ) / 2.0;

    L = sqrt(
        pow( result_array[0]->x - center.x, 2 )
      + pow( result_array[0]->y - center.y, 2 )
    );

    Um = (
            -( result_array[0]->x - result_array[2]->x )
           / ( result_array[0]->y - result_array[2]->y )
         );

    Ub = center.y - center.x * Um;

    elog( DEBUG1, "Desired slope Um is %f", Um );
    if( fabs( result_array[0]->x - result_array[2]->x ) < DBL_EPSILON )
    {
        // Diag slope is inf
        result_array[1]->x = center.x + L;
        result_array[1]->y = center.y;
        result_array[3]->x = center.x - L;
        result_array[3]->y = center.y;

    }
    else if( fabs( result_array[0]->y - result_array[2]->y ) < DBL_EPSILON )
    {
        // Diag slope is 0
        result_array[1]->x = center.x;
        result_array[1]->y = center.y + L;
        result_array[3]->x = center.x;
        result_array[3]->y = center.y - L;
    }
    else
    {
        a = pow( Um, 2 ) + 1.0;
        b = 2.0 * ( Ub * Um - center.y * Um - center.x );
        c = pow( Ub, 2 ) - pow( L, 2 ) - 2.0 * center.y * Ub
          + pow( center.x, 2 ) + pow( center.y, 2 );

        if( ( b * b ) < ( 4 * a * c ) )
        {
            __degenerate_solution( a, b, c );

            for( i = 0; i < 4; i++ )
            {
                pfree( result_array[i] );
            }

            pfree( result_array );
            return NULL;
        }

        result_array[1]->x = ( -b + sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
        result_array[3]->x = ( -b - sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
        result_array[1]->y = Um * result_array[1]->x + Ub;
        result_array[3]->y = Um * result_array[3]->x + Ub;
    }

    return result_array;
}

LSEG ** get_box_lsegs( BOX * b )
{
    LSEG **      result      = NULL;
    Point **     lseg_points = NULL;
    unsigned int i           = 0;
    unsigned int next_i      = 0;

    if( b == NULL )
    {
        return NULL;
    }

    lseg_points = get_box_points( b );

    if( lseg_points == NULL )
    {
        return NULL;
    }

    result = ( LSEG ** ) palloc0( sizeof( LSEG * ) * 4 );

    if( result == NULL )
    {
        pfree( lseg_points );
        return NULL;
    }

    for( i = 0; i < 4; i++ )
    {
        if( i == 3 )
        {
            next_i = 0;
        }
        else
        {
            next_i = i + 1;
        }

        result[i]->p[0].x = lseg_points[i]->x;
        result[i]->p[0].y = lseg_points[i]->y;
        result[i]->p[1].x = lseg_points[next_i]->x;
        result[i]->p[1].y = lseg_points[next_i]->y;
    }

    for( i = 0; i < 4; i++ )
    {
        pfree( lseg_points[i] );
    }

    pfree( lseg_points );

    return result;
}

