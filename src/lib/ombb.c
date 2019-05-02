/*------------------------------------------------------------------------------
 * ombb.c
 *     Implementation of Oriented Minimum Bounding Box using the rotating
 *     calipers algorithm. This is an O(n^2) naive implementation that can
 *     certainly be improved upon.
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      ombb.c
 *
 *------------------------------------------------------------------------------
 */

#include "ombb.h"

Point ** get_ombb(
    POLYGON * p,
    double *  o_len_o,
    double *  o_len_p,
    double *  o_theta,
    bool      discard_points
)
{
    Point **     result      = NULL;
    unsigned int i           = 0;
    unsigned int j           = 0;
    double       best_area   = DBL_MAX;
    Point        center      = {0.0}; // Center point of best rect
    Point        unit_p      = {0.0}; // Parallel unit vector
    Point        unit_o      = {0.0}; // Orthogonal unit vector
    double       dp_min_p    = 0.0; // Dot product data
    double       dp_min_o    = 0.0;
    double       dp_max_p    = 0.0;
    double       dp_max_o    = 0.0;
    double       dp          = 0.0;
    double       len_p       = 0.0;
    double       len_o       = 0.0;
    double       theta       = 0.0; // For rotation post-processing
    double       diff_angle  = 0.0;
    double       diff_length = 0.0;
    double       x           = 0.0;
    double       y           = 0.0;

    if( p == NULL )
    {
        return NULL;
    }

    result = ( Point ** ) palloc0( sizeof( Point * ) * 4 );

    if( result == NULL )
    {
        return NULL;
    }

    for( i = 0; i < 4; i++ )
    {
        result[i] = ( Point * ) palloc0( sizeof( Point ) );

        if( result[i] == NULL )
        {
            for( j = i; i > 0; j-- )
            {
                pfree( result[j] );
            }

            pfree( result );
        }
    }

    for( i = 0; i < p->npts; i++ )
    {
        // Normalize the edge vectors
        unit_p = _unit_vector( p->p[i], p->p[(i + 1) % p->npts] );
        unit_o = _orthogonal_vector( unit_p );

        dp_min_p = DBL_MAX;
        dp_max_p = -DBL_MAX;

        for( j = 0; j < p->npts; j++ )
        {
            // Skip the vertex currently under inspection
            if( i == j )
            {
                continue;
            }

            dp = dot_product( &unit_p, &(p->p[j]) );

            if( dp < dp_min_p )
            {
                dp_min_p = dp;
            }
            else if( dp > dp_max_p )
            {
                dp_max_p = dp;
            }
        }

        dp_min_o = DBL_MAX;
        dp_max_o = -DBL_MAX;

        for( j = 0; j < p->npts; j++ )
        {
            // Skip the vertex currently under inspection
            if( i == j )
            {
                continue;
            }

            dp = dot_product( &unit_o, &(p->p[j]) );

            if( dp < dp_min_o )
            {
                dp_min_o = dp;
            }
            else if( dp > dp_max_o )
            {
                dp_max_o = dp;
            }
        }

        len_p = ( dp_max_p - dp_min_p );
        len_o = ( dp_max_o - dp_min_o );

        if( // Disallow results that are of 0 area or -inf area
              ( len_p * len_o ) < best_area
           && !( fabs( len_p * len_o ) < DBL_EPSILON )
           && !( fabs( len_p * len_o ) >= DBL_MAX )
          )
        {
            best_area    = len_p * len_o;
            x            = dp_min_p + ( len_p / 2.0 );
            y            = dp_min_o + ( len_o / 2.0 );
            theta        = atan2( unit_p.y, unit_p.x );
            center.x     = x * cos( theta ) - y * sin( theta );
            center.y     = x * sin( theta ) + y * cos( theta );
            result[0]->x = center.x + 0.5 * len_p;
            result[1]->x = center.x + 0.5 * len_p;
            result[2]->x = center.x - 0.5 * len_p;
            result[3]->x = center.x - 0.5 * len_p;
            result[0]->y = center.y + 0.5 * len_o;
            result[1]->y = center.y - 0.5 * len_o;
            result[2]->y = center.y - 0.5 * len_o;
            result[3]->y = center.y + 0.5 * len_o;

            // Write output data if caller requested it
            if( o_len_o != NULL )
            {
                *o_len_o = len_o;
            }

            if( o_len_p != NULL )
            {
                *o_len_p = len_p;
            }

            if( o_theta != NULL )
            {
                *o_theta = theta;
            }

            // Rotate the points about the fixed center and the angle of
            // the parallel unit vector (unit_p) such that the output
            // is oriented with the input
            for( j = 0; j < 4; j++ )
            {
                diff_angle  = atan2(
                                 result[j]->y - center.y,
                                 result[j]->x - center.x
                              ) + theta;
                diff_length = sqrt(
                                  pow( result[j]->y - center.y, 2 )
                                + pow( result[j]->x - center.x, 2 )
                              );
                result[j]->x = center.x + diff_length * cos( diff_angle );
                result[j]->y = center.y + diff_length * sin( diff_angle );

                if( fabs( result[j]->x ) < DBL_EPSILON )
                {
                    result[j]->x = 0.0;
                }

                if( fabs( result[j]->y ) < DBL_EPSILON )
                {
                    result[j]->y = 0.0;
                }
            }
        }
    }

    if( fabs( best_area - DBL_MAX ) < DBL_EPSILON || discard_points == true )
    {
        for( i = 0; i < 4; i++ )
        {
            pfree( result[i] );
        }

        pfree( result[i] );
        return NULL;
    }

    return result;
}

inline Point _unit_vector( Point a, Point b )
{
    Point  result   = {0.0};
    double distance = 0.0;

    distance = sqrt(
                   pow( a.x - b.x, 2 )
                 + pow( a.y - b.y, 2 )
               );
    result.x = ( b.x - a.x ) / distance;
    result.y = ( b.y - a.y ) / distance;

    return result;
}

inline Point _orthogonal_vector( Point a )
{
    Point result = { -1.0 * a.y, a.x };

    return result;
}
