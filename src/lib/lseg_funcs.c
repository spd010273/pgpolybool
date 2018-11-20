/*------------------------------------------------------------------------------
 * lseg_funcs.c
 *      PostgreSQL LSEG helper function declarations
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      lseg_funcs.c
 *
 *------------------------------------------------------------------------------
 */

#include "lseg_funcs.h"

bool line_segment_intersect( LSEG * a, LSEG * b )
{
    struct segment * mp_sega     = NULL;
    struct segment * mp_segb     = NULL;
    Point *          isect_p0    = NULL;
    Point *          isect_p1    = NULL;
    unsigned int     isect_count = 0;

    mp_sega = new_segment();
    mp_segb = new_segment();

    segment_set_begin( mp_sega, &(a->p[0]) );
    segment_set_end( mp_sega, &(a->p[1]) );
    segment_set_begin( mp_segb, &(b->p[0]) );
    segment_set_end( mp_segb, &(b->p[0]) );

    isect_p0 = ( Point * ) palloc0( sizeof( Point ) );
    isect_p1 = ( Point * ) palloc0( sizeof( Point ) );

    if( isect_p0 == NULL || isect_p1 == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Failed to allocate intersection points" )
            )
        );
    }

    isect_count = find_intersection( mp_sega, mp_segb, isect_p0, isect_p1 );

    pfree( isect_p0 );
    pfree( isect_p1 );
    pfree( mp_sega );
    pfree( mp_segb );

    if( isect_count == 0 )
    {
        return false;
    }

    return true;
}

Point * line_segment_intersection( LSEG * a, LSEG * b )
{
    struct segment * mp_sega     = NULL;
    struct segment * mp_segb     = NULL;
    Point *          isect_p0    = NULL;
    Point *          isect_p1    = NULL;
    unsigned int     isect_count = 0;

    if( a == NULL || b == NULL )
    {
        return NULL;
    }

    mp_sega = new_segment();
    mp_segb = new_segment();

    segment_set_begin( mp_sega, &(a->p[0]) );
    segment_set_end( mp_sega, &(a->p[1]) );
    segment_set_begin( mp_segb, &(b->p[0]) );
    segment_set_end( mp_segb, &(b->p[0]) );

    isect_p0 = ( Point * ) palloc0( sizeof( Point ) );
    isect_p1 = ( Point * ) palloc0( sizeof( Point ) );

    if( isect_p0 == NULL || isect_p1 == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Failed to allocate intersection points" )
            )
        );
    }

    isect_count = find_intersection( mp_sega, mp_segb, isect_p0, isect_p1 );

    pfree( isect_p1 );
    pfree( mp_sega );
    pfree( mp_segb );

    if( isect_count == 0 )
    {
        return NULL;
    }

    return isect_p0;
}

double line_segment_distance( LSEG * l1, LSEG * l2 )
{
    double  mag_A         = 0.0;
    double  mag_B         = 0.0;
    double  cross_product = 0.0;
    double  denominator   = 0.0;
    double  dot           = 0.0;
    double  d0            = 0.0;
    double  d1            = 0.0;
    double  det_a         = 0.0;
    double  det_b         = 0.0;
    double  t0            = 0.0;
    double  t1            = 0.0;
    double  dist          = 0.0;
    Point * temp          = NULL;
    Point * A             = NULL;
    Point * B             = NULL;
    Point * _A            = NULL;
    Point * _B            = NULL;
    Point * t             = NULL;
    Point * proj_A        = NULL;
    Point * proj_B        = NULL;

    A      = ( Point * ) palloc0( sizeof( Point ) );
    B      = ( Point * ) palloc0( sizeof( Point ) );
    _A     = ( Point * ) palloc0( sizeof( Point ) );
    _B     = ( Point * ) palloc0( sizeof( Point ) );
    temp   = ( Point * ) palloc0( sizeof( Point ) );
    t      = ( Point * ) palloc0( sizeof( Point ) );
    proj_A = ( Point * ) palloc0( sizeof( Point ) );
    proj_B = ( Point * ) palloc0( sizeof( Point ) );

    if(
            A == NULL || B == NULL || _A == NULL || _B == NULL
         || temp == NULL || t == NULL || proj_A == NULL || proj_B == NULL
      )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Failed to allocate delta or projection points" )
            )
        );
    }

    // Calculate denomitator
    A->x = l1->p[1].x - l1->p[0].x;
    A->y = l1->p[1].y - l1->p[0].y;
    B->x = l2->p[1].x - l2->p[0].x;
    B->y = l2->p[1].y - l2->p[0].y;

    // Get magnitudes
    mag_A = sqrt( dot_product( A, A ) );
    mag_B = sqrt( dot_product( B, B ) );

    _A->x = A->x / mag_A;
    _A->y = A->y / mag_A;
    _B->x = B->x / mag_B;
    _B->y = B->y / mag_B;

    cross_product = _A->x * _B->y - _A->y * _B->x;
    denominator   = sqrt( cross_product * cross_product );
    denominator   = denominator * denominator;


    // If lines are parallel (denom=0) test if lines overlap.
    // If they don't overlap then there is a closest point solution.
    // If they do overlap, there are infinite closest positions, but there is a closest distance
    if( denominator == 0 )
    {
        temp->x = l2->p[0].x - l1->p[0].x;
        temp->y = l2->p[0].y - l1->p[0].y;
        d0      = dot_product( _A, temp );

        temp->x = l2->p[1].x - l1->p[0].x;
        temp->y = l2->p[1].y - l1->p[0].y;
        d1      = dot_product( _A, temp );

        // Is segment B before A?
        if( d0 <= 0 && d1 <= 0 )
        {
            pfree( A );
            pfree( _A );
            pfree( B );
            pfree( _B );
            pfree( temp );

            if( fabs( d0 ) < fabs( d1 ) )
            {
                t0   = l1->p[0].x - l2->p[0].x;
                t1   = l1->p[0].y - l2->p[0].y;
                dist = sqrt( t0 * t0 + t1 * t1 );

                return dist;
            }

            t0   = l1->p[0].x - l2->p[1].x;
            t1   = l1->p[0].y - l2->p[1].y;
            dist = sqrt( t0 * t0 + t1 * t1 );

            return dist;
        }
        else if( d0 >= mag_A && mag_A <= d1 )
        {
            pfree( A );
            pfree( _A );
            pfree( B );
            pfree( _B );
            pfree( temp );

            // Is segment B after A?
            if( fabs( d0 ) < fabs( d1 ) )
            {
                t0   = l1->p[1].x - l2->p[0].x;
                t1   = l1->p[1].y - l2->p[0].y;
                dist = sqrt( t0 * t0 + t1 * t1 );

                return dist;
            }

            t0   = l1->p[1].x - l2->p[1].x;
            t1   = l1->p[1].y - l2->p[1].y;
            dist = sqrt( t0 * t0 + t1 * t1 );

            return dist;
        }

        // Segments overlap, return distance between parallel segments
        t0   = d0 * _A->x + l1->p[0].x - l2->p[0].x;
        t1   = d0 * _A->y + l1->p[0].y - l2->p[0].y;
        dist = sqrt( t0 * t0 + t1 * t1 );

        pfree( A );
        pfree( _A );
        pfree( B );
        pfree( _B );
        pfree( temp );

        return dist;
    }



    // Lines intersect: Calculate the projected closest points
    t->x = l2->p[0].x - l1->p[0].x;
    t->y = l2->p[0].y - l1->p[0].y;

    det_a = t->x * ( _B->y - 1 )
          - _B->x * ( t->y - 1 )
          + cross_product * ( t->y - _B->y );
    det_b = t->x * ( _A->y - 1 )
          - _A->x * ( t->y - 1 )
          + cross_product * ( t->y - _A->y );

    t0 = det_a / denominator;
    t1 = det_b / denominator;

    // Projected closest points
    proj_A->x = l1->p[0].x + ( _A->x * t0 );
    proj_A->y = l1->p[0].y + ( _A->y * t0 );
    proj_B->x = l2->p[0].x + ( _B->x * t1 );
    proj_B->y = l2->p[0].y + ( _B->y * t1 );

    if( t0 < 0 )
    {
        proj_A->x = l1->p[0].x;
        proj_A->y = l1->p[0].y;
    }
    else if( t0 > mag_A )
    {
        proj_A->x = l1->p[1].x;
        proj_A->y = l1->p[1].y;
    }

    if( t1 < 0 )
    {
        proj_B->x = l2->p[0].x;
        proj_B->y = l2->p[0].y;
    }
    else if( t1 > mag_B )
    {
        proj_B->x = l2->p[1].x;
        proj_B->y = l2->p[1].y;
    }

    if( t0 < 0 || t0 > mag_A )
    {
        temp->x = proj_A->x - l2->p[0].x;
        temp->y = proj_A->y - l2->p[0].y;
        dot     = dot_product( _B, temp );

        if( dot < 0 )
        {
            dot = 0;
        }
        else if( dot > mag_B )
        {
            dot = mag_B;
        }

        proj_B->x = l2->p[0].x + ( _B->x * dot );
        proj_B->y = l2->p[0].y + ( _B->y * dot );
    }

    if( t1 < 0 || t1 > mag_B )
    {
        temp->x = proj_B->x - l1->p[0].x;
        temp->y = proj_B->y - l1->p[0].y;
        dot     = dot_product( _A, temp );

        if( dot < 0 )
        {
            dot = 0;
        }
        else if( dot > mag_A )
        {
            dot = mag_A;
        }

        proj_A->x = l1->p[0].x + ( _A->x * dot );
        proj_A->y = l1->p[0].y + ( _A->y * dot );
    }

    t0   = proj_A->x - proj_B->x;
    t1   = proj_A->y - proj_B->y;
    dist = sqrt( t0 * t0 + t1 * t1 );

    pfree( proj_A );
    pfree( proj_B );
    pfree( temp );
    pfree( _A );
    pfree( A );
    pfree( _B );
    pfree( B );
    pfree( t );

    return dist;
}

LSEG * line_segment_orthogonal_line_segment( LSEG * segment, Point * away_point, double length )
{
    LSEG * result = NULL;
    double A      = 0.0;
    double B      = 0.0;
    double C      = 0.0;
    double D      = 0.0;
    double E      = 0.0;
    double F      = 0.0;
    double curr_s = 0.0; // For orth slope
    double targ_s = 0.0;
    double y_int  = 0.0;
    double d1     = 0.0; // For distance comp
    double d2     = 0.0;
    double a      = 0.0; // For quadratic solution
    double b      = 0.0;
    double c      = 0.0;
    double x1     = 0.0;
    double x2     = 0.0;
    double y1     = 0.0;
    double y2     = 0.0;

    if( segment == NULL )
    {
        return NULL;
    }

    result = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( result == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create output segment" )
            )
        );
    }

    A = segment->p[0].x;
    B = segment->p[0].y;
    C = segment->p[1].x;
    D = segment->p[1].y;

    result->p[0].x = ( A + C ) / 2;
    result->p[0].y = ( B + D ) / 2;

    E = result->p[0].x;
    F = result->p[0].y;

    if( A == C ) // Current slope is inf, target slope is 0
    {
        result->p[1].y = F;

        if( away_point != NULL )
        {
            result->p[1].x = E + length;
            d1 = distance( away_point, &(result->p[1]) );
            result->p[1].x = E - length;
            d2 = distance( away_point, &(result->p[1]) );

            if( d1 > d2 )
            {
                result->p[1].x = E + length;
            }
        }
        else
        {
            result->p[1].x = E + length;
        }
    }
    else if( B == D ) // Current slope is 0, target_slope is inf
    {
        result->p[1].x = E;

        if( away_point != NULL )
        {
            result->p[1].y = F + length;
            d1 = distance( away_point, &(result->p[1]) );
            result->p[1].y = F - length;
            d2 = distance( away_point, &(result->p[1]) );

            if( d1 > d2 )
            {
                result->p[1].y = F + length;
            }
        }
        else
        {
            result->p[1].y = F + length;
        }
    }
    else
    {
        // Solve for y = mx + b
        curr_s = ( A - C ) / ( B - D );
        targ_s = pow( curr_s, -1 ) * -1;
        y_int  = F - ( E * targ_s );

        // Find quadratic solution to sqrt( ( E - x )^2 + ( F - y )^2 ) = length
        a = 1 + ( targ_s * targ_s );
        b = ( 2 * targ_s * y_int ) - ( 2 * F * targ_s ) - ( 2 * E );
        c = ( E * E ) + ( F * F ) - ( 2 * F * y_int ) + ( y_int * y_int ) - ( length * length );

        if( ( b * b ) < ( 4 * a * c ) )
        {
            elog( DEBUG1, "solution for quadratic a=%f, b=%f, c=%f is degenerate", a, b, c );
            return NULL;
        }

        x1 = ( -b + sqrt( ( b * b ) - ( 4 * a * c ) ) ) / ( 2 * a );
        x2 = ( -b - sqrt( ( b * b ) - ( 4 * a * c ) ) ) / ( 2 * a );
        y1 = targ_s * x1 + y_int;
        y2 = targ_s * x2 + y_int;

        result->p[1].x = x1;
        result->p[1].y = y1;

        if( away_point != NULL )
        {
            d1 = distance( away_point, &(result->p[1]) );
            result->p[1].x = x2;
            result->p[1].y = y2;
            d2 = distance( away_point, &(result->p[1]) );

            if( d1 > d2 )
            {
                result->p[1].x = x1;
                result->p[1].y = y1;
            }
        }
    }

    return result;
}
