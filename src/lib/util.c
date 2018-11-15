/*------------------------------------------------------------------------------
 * util.c
 *     Point math and helper functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      util.c
 *
 *------------------------------------------------------------------------------
 */

#include "util.h"

double signed_area_three( Point * a, Point * b, Point * c )
{
    return ( a->x - c->x ) * ( b->y - c->y )
         - ( b->x - c->x ) * ( a->y - c->y );
}

double signed_area_two( Point * a, Point * b )
{
    if( a == NULL || b == NULL )
    {
        return 0;
    }

    return -b->x * ( a->y - b->y ) - -b->y * ( a->x - b->x );
}

int sign( Point * a, Point * b, Point * c )
{
    double determinant = 0.0;

    if( a == NULL || b == NULL || c == NULL )
    {
        return 0;
    }

    determinant = signed_area_three( a, b, c );

    if( determinant < 0.0 )
    {
        return -1;
    }

    if( determinant > 0.0 )
    {
        return 1;
    }

    return 0;
}

bool point_in_triangle( struct segment * s, Point * a, Point * b )
{
    int x = 0;

    if( s == NULL || a == NULL || b == NULL )
    {
        return false;
    }

    x = sign( s->p1, s->p2, b );

    if( x == sign( s->p2, a, b ) && x == sign( a, s->p1, b ) )
    {
        return true;
    }

    return false;
}

double distance( Point * a, Point * b )
{
    double distance = 0.0;
    double dx       = 0.0;
    double dy       = 0.0;

    if( a == NULL || b == NULL )
    {
        return distance;
    }

    dx = a->x - b->x;
    dy = a->y - b->y;

    distance = sqrt( dx * dx + dy * dy );

    return distance;
}

unsigned int find_intersection(
    struct segment * s0,
    struct segment * s1,
    Point *          p0, // pi0
    Point *          p1 // pi1
)
{
    Point *      isect_s0   = NULL; // p0
    Point *      isect_s1   = NULL; // p1
    Point *      d0         = NULL; // do
    Point *      d1         = NULL; // d1
    Point *      E          = NULL;
    double       cross_prod = 0.0;
    double       sq_len0    = 0.0;
    double       sq_len1    = 0.0;
    double       sq_len_E   = 0.0;
    double       sq_epsilon = 0.0;
    double       s          = 0.0;
    double       t          = 0.0;
    double       s_0        = 0.0;
    double       s_1        = 0.0;
    double       s_min      = 0.0;
    double       s_max      = 0.0;
    double       w[2]       = {0.0};
    unsigned int imax       = 0;

    sq_epsilon = 0.0000001;

    if( s0 == NULL || s1 == NULL )
    {
        return 0;
    }

    isect_s0 = s0->p1;
    isect_s1 = s1->p1;

    d0 = ( Point * ) palloc0( sizeof( Point ) );
    d1 = ( Point * ) palloc0( sizeof( Point ) );
    E  = ( Point * ) palloc0( sizeof( Point ) );

    if( d0 == NULL || d1 == NULL || E == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg(
                    "Could not create reference points"\
                    " for intersection calculation"
                )
            )
        );
    }

    d0->x = s0->p2->x - s0->p1->x;
    d0->y = s0->p2->y - s0->p1->y;

    d1->x = s1->p2->x - s1->p1->x;
    d1->y = s1->p2->y - s1->p1->y;

    E->x = isect_s1->x - isect_s0->x;
    E->y = isect_s1->y - isect_s0->y;

    cross_prod = d0->x * d1->y - d0->y * d1->x;
    sq_len0    = d0->x * d0->x + d0->y * d0->y;
    sq_len1    = d1->x * d1->x + d1->y * d1->y;

    // Handle the non-parallel case
    if( ( cross_prod * cross_prod ) > ( sq_epsilon * sq_len0 * sq_len1 ) )
    {
        s = ( E->x * d1->y - E->y * d1->x ) / cross_prod;

        if( s < 0 || s > 1 )
        {
            pfree( d0 );
            pfree( d1 );
            pfree( E );
            return 0;
        }

        t = ( E->x * d0->y - E->y * d0->x ) / cross_prod;

        if( t < 0 || t > 1 )
        {
            pfree( d0 );
            pfree( d1 );
            pfree( E );
            return 0;
        }

        p0->x = isect_s0->x + s * d0->x;
        p0->y = isect_s0->y + s * d0->y;

        pfree( d0 );
        pfree( d1 );
        pfree( E );

        if( distance( p0, s0->p1 ) < sq_epsilon )
        {
            p0 = s0->p1;
        }

        if( distance( p0, s0->p2 ) < sq_epsilon )
        {
            p0 = s0->p2;
        }

        if( distance( p0, s1->p1 ) < sq_epsilon )
        {
            p0 = s1->p1;
        }

        if( distance( p0, s1->p2 ) < sq_epsilon )
        {
            p0 = s1->p2;
        }

        return 1;
    }

    // Handle the parallel case
    sq_len_E   = E->x * E->x + E->y * E->y;
    cross_prod = E->x * d0->y - E->y * d0->x;

    if( ( cross_prod * cross_prod ) > ( sq_epsilon * sq_len0 * sq_len_E ) )
    {
        pfree( d0 );
        pfree( d1 );
        pfree( E );
        return 0;
    }

    // Segments are colinear but we need to test for an overlap of the segments
    s_0 = ( d0->x * E->x + d0->y * E->y ) / sq_len0;
    s_1 = s_0 + ( d0->x * d1->x + d0->y * d1->y ) / sq_len0;

    s_min = ( s_0 < s_1 ) ? s_0 : s_1;
    s_max = ( s_0 > s_1 ) ? s_0 : s_1;

    // u1 == 0.0, u1 == 1.0, v0 == smin, v1 == smax
    if( 1.0 < s_min || 0.0 > s_max )
    {
        return 0;
    }
    else
    {
        if( 1.0 > s_min )
        {
            if( 0.0 < s_max )
            {
                if( 0.0 < s_min )
                {
                    w[0] = s_min;
                }
                else
                {
                    w[0] = 0.0;
                }

                if( 1.0 > s_max )
                {
                    w[1] = s_max;
                }
                else
                {
                    w[1] = 1.0;
                }

                imax = 2;
            }
            else
            {
                w[0] = 0.0;
                imax = 1;
            }
        }
        else
        {
            w[0] = 1.0;
            imax = 1;
        }
    }

    if( imax > 0 )
    {
        p0->x = isect_s0->x + w[0] * d0->x;
        p0->y = isect_s1->y + w[0] * d0->y;

        if( distance( p0, s0->p1 ) < sq_epsilon )
        {
            p0 = s0->p1;
        }

        if( distance( p0, s0->p2 ) < sq_epsilon )
        {
            p0 = s0->p2;
        }

        if( distance( p0, s1->p1 ) < sq_epsilon )
        {
            p0 = s1->p1;
        }

        if( distance( p0, s1->p2 ) < sq_epsilon )
        {
            p0 = s1->p2;
        }

        if( imax > 1 )
        {
            p1->x = p0->x + w[1] * d0->x;
            p1->y = p0->y + w[1] * d0->y;
        }
    }

    pfree( d0 );
    pfree( d1 );
    pfree( E );
    return imax;
}

bool points_equal( Point * a, Point * b )
{
    if( a == NULL || b == NULL )
    {
        return false;
    }

    if( a->x == b->x && a->y == b->y )
    {
        return true;
    }

    return false;
}

double dot_product( Point * a, Point * b )
{
    if( a == NULL || b == NULL )
    {
        elog( DEBUG1, "Dot product inputs are null!" );
        return 0.0;
    }

    return ( a->x * b->x + a->y * b->y );
}

bool _fp_gte( double a, double b )
{
    if( a < b )
    {
        return false;
    }

    return true;
}

bool _fp_gt( double a, double b )
{
    if( a > b && !_fp_eq( a, b ) )
    {
        return true;
    }

    return false;
}

bool _fp_lte( double a, double b )
{
    if( a > b )
    {
        return false;
    }

    return true;
}

bool _fp_lt( double a, double b )
{
    if( a < b && !_fp_eq( a, b ) )
    {
        return true;
    }

    return false;
}

bool _fp_eq( double a, double b )
{
    double diff = 0.0;
    double min  = 0.0;
    double a_a  = 0.0;
    double a_b  = 0.0;

    // Exact equality
    if( a == b )
    {
        return true;
    }

    diff = fabs( a - b );
    // Both == 0?
    if( a == 0 || b == 0 || diff <= DBL_EPSILON )
    {
        return true;
    }

    // FP error-based approximation of equality
    a_a = fabs( a );
    a_b = fabs( b );

    if( ( a_a + a_b ) > DBL_MAX )
    {
        min = DBL_MAX;
    }
    else
    {
        min = a_a + a_b;
    }

    if( ( diff / min ) < DBL_EPSILON )
    {
        return true;
    }

    return false;
}
