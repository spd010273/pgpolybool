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
        __oom( "Failed to allocate intersection points" );
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
        __oom( "Failed to allocate intersection points" );
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
    double  cross_prod    = 0.0;
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
        __oom( "Failed to allocate delta or projection points" );
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

    cross_prod  = cross_product( _A, _B );
    denominator = sqrt( cross_prod * cross_prod );
    denominator = denominator * denominator;

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
          + cross_prod * ( t->y - _B->y );
    det_b = t->x * ( _A->y - 1 )
          - _A->x * ( t->y - 1 )
          + cross_prod * ( t->y - _A->y );

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

LSEG ** line_segment_parallel_line_segment(
    LSEG *  segment,
    Point * away_point,
    double  line_distance
)
{
    LSEG **      result     = NULL;
    double       curr_slope = 0.0; // input line slope
    double       targ_slope = 0.0; // output line slope
    double       y_int_0    = 0.0; // y int for point 0 of input
    double       y_int_1    = 0.0; // y int for point 1 of input
    double       d1         = 0.0; // For distance comparison w/ away_point
    double       d2         = 0.0;
    double       a          = 0.0; // For quadratic solution
    double       b_0        = 0.0; // quadratic b for point 0
    double       c_0        = 0.0; // quadratic b for point 1
    double       b_1        = 0.0; // quadratic c for point 0
    double       c_1        = 0.0; // quadratic c for point 1
    double       x1_0       = 0.0; // Solution 1 point 0
    double       y1_0       = 0.0;
    double       x1_1       = 0.0; // Solution 1 point 1
    double       y1_1       = 0.0;
    double       x2_0       = 0.0; // Solution 2 point 0
    double       y2_0       = 0.0;
    double       x2_1       = 0.0; // Solution 2 point 1
    double       y2_1       = 0.0;
    unsigned int size       = 0;

    if( segment == NULL )
    {
        return NULL;
    }

    size = 1;

    if( away_point == NULL )
    {
        size++;
    }

    result = ( LSEG ** ) palloc0( sizeof( LSEG * ) * size );

    if( result == NULL )
    {
        __oom( "Could not create output segment" );
    }

    result[0] = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( away_point == NULL )
    {
        result[1] = ( LSEG * ) palloc0( sizeof( LSEG ) );

        if( result[1] == NULL )
        {
            __oom( "Could not create orthogonal root" );
        }
    }

    if( result[0] == NULL )
    {
        __oom( "Could not create orthogonal root" );
    }

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Current slope is infinite (vertical line)
        result[0]->p[0].y = segment->p[0].y;
        result[0]->p[1].y = segment->p[1].y;

        if( away_point != NULL )
        {
            result[0]->p[0].x = segment->p[0].x + line_distance;
            d1 = distance( away_point, (&(result[0]->p[0])) );
            result[0]->p[0].x = segment->p[0].x - line_distance;
            d2 = distance( away_point, (&(result[0]->p[0])) );

            if( d1 > d2 )
            {
                result[0]->p[0].x = segment->p[0].x + line_distance;
                result[0]->p[1].x = segment->p[1].x + line_distance;
            }
            else
            {
                result[0]->p[1].x = segment->p[1].x - line_distance;
            }

        }
        else
        {
            result[1]->p[0].y = segment->p[0].y;
            result[1]->p[1].y = segment->p[1].y;

            result[0]->p[0].x = segment->p[0].x - line_distance;
            result[0]->p[1].x = segment->p[1].x - line_distance;
            result[1]->p[0].x = segment->p[0].x + line_distance;
            result[1]->p[1].x = segment->p[1].x + line_distance;
        }
    }
    else if( fabs( segment->p[0].y - segment->p[1].y ) < DBL_EPSILON )
    {
        // Current slope is 0 (horizontal line)
        result[0]->p[0].x = segment->p[0].x;
        result[0]->p[1].x = segment->p[1].x;

        if( away_point != NULL )
        {
            result[0]->p[0].y = segment->p[0].y + line_distance;
            d1 = distance( away_point, (&(result[0]->p[0])) );
            result[0]->p[0].y = segment->p[0].y - line_distance;
            d2 = distance( away_point, (&(result[0]->p[0])) );

            if( d1 > d2 )
            {
                result[0]->p[0].y = segment->p[0].y + line_distance;
                result[0]->p[1].y = segment->p[1].y + line_distance;
            }
            else
            {
                result[0]->p[1].y = segment->p[1].y - line_distance;
            }

        }
        else
        {
            result[1]->p[0].x = segment->p[0].x;
            result[1]->p[1].x = segment->p[1].x;

            result[0]->p[0].y = segment->p[0].y - line_distance;
            result[0]->p[1].y = segment->p[1].y - line_distance;
            result[1]->p[0].y = segment->p[0].y + line_distance;
            result[1]->p[1].y = segment->p[1].y + line_distance;
        }
    }
    else
    {
        // Solve for y = mx + b for a line extending orthogonally from one of the input's endpoints
        curr_slope = ( segment->p[0].y - segment->p[1].y ) / ( segment->p[0].x - segment->p[1].x );
        targ_slope = -1.0 / curr_slope;
        y_int_0    = segment->p[0].y - ( segment->p[0].x * targ_slope );
        y_int_1    = segment->p[1].y - ( segment->p[1].x * targ_slope );

        // Find quadratic solution to sqrt( ( endpoint_x - x )^2 + ( endpoint_y - y )^2 ) = length
        // Plugging back in y = mx + b for both the unknown (y) and known (endpoint_y) values,
        // this factors to the following equation:
        // 0 = ( 1 + targ_slope^2 )( x^2 - 2 * endpoint_x * x + endpoint_x^2 ) - line_distance^2
        // Which feeds into the following quadratic:
        a   = 1.0 + ( targ_slope * targ_slope );

        b_0 = -2.0 * a * segment->p[0].x;
        c_0 = a * segment->p[0].x * segment->p[0].x - line_distance * line_distance;

        b_1 = -2.0 * a * segment->p[1].x;
        c_1 = a * segment->p[1].x * segment->p[1].x - line_distance * line_distance;

        if( pow( b_0, 2 ) < ( 4 * a * c_0 ) )
        {
            __degenerate_solution( a, b_0, c_0 );
            pfree( result[0] );

            if( away_point == NULL )
            {
                pfree( result[1] );
            }

            pfree( result );
            return NULL;
        }

        if( pow( b_1, 2 ) < ( 4 * a * c_1 ) )
        {
            __degenerate_solution( a, b_1, c_1 );
            pfree( result[0] );

            if( away_point == NULL )
            {
                pfree( result[1] );
            }

            pfree( result );
            return NULL;
        }

        // coordinate<solution #>_<point_index>

        x1_0 = ( -b_0 + sqrt( ( b_0 * b_0 ) - ( 4 * a * c_0 ) ) ) / ( 2 * a );
        y1_0 = targ_slope * x1_0 + y_int_0;

        x2_0 = ( -b_0 - sqrt( ( b_0 * b_0 ) - ( 4 * a * c_0 ) ) ) / ( 2 * a );
        y2_0 = targ_slope * x2_0 + y_int_0;

        x1_1 = ( -b_1 + sqrt( ( b_1 * b_1 ) - ( 4 * a * c_1 ) ) ) / ( 2 * a );
        y1_1 = targ_slope * x1_1 + y_int_1;

        x2_1 = ( -b_1 - sqrt( ( b_1 * b_1 ) - ( 4 * a * c_1 ) ) ) / ( 2 * a );
        y2_1 = targ_slope * x2_1 + y_int_1;

        result[0]->p[0].x = x1_0;
        result[0]->p[0].y = y1_0;
        result[0]->p[1].x = x1_1;
        result[0]->p[1].y = y1_1;

        // Similar logic as the special case solution, find the vector end
        // farthest from away_point
        if( away_point != NULL )
        {
            d1 = distance( away_point, (&(result[0]->p[0])) );
            result[0]->p[0].x = x2_0;
            result[0]->p[0].y = y2_0;
            d2 = distance( away_point, (&(result[0]->p[0])) );

            if( d1 > d2 )
            {
                // Keep solution originally assigned
                result[0]->p[0].x = x1_0;
                result[0]->p[0].y = y1_0;
            }
            else
            {
                // Swap solutions
                result[0]->p[0].x = x2_0;
                result[0]->p[0].y = y2_0;
                result[0]->p[1].x = x2_1;
                result[0]->p[1].y = y2_1;
            }
        }
        else
        {
            result[1]->p[0].x = x2_0;
            result[1]->p[0].y = y2_0;
            result[1]->p[1].x = x2_1;
            result[1]->p[1].y = y2_1;
        }
    }

    return result;
}

LSEG ** line_segment_orthogonal_line_segment(
    LSEG *  segment,
    Point * away_point,
    double  length
)
{
    LSEG **      result     = NULL;
    double       curr_slope = 0.0; // input line slope
    double       targ_slope = 0.0; // output line slope
    double       y_int      = 0.0;
    double       d1         = 0.0; // For distance comp
    double       d2         = 0.0;
    double       a          = 0.0; // For quadratic solution
    double       b          = 0.0;
    double       b2         = 0.0;
    double       c          = 0.0;
    double       x1         = 0.0; // For result
    double       x2         = 0.0;
    double       y1         = 0.0;
    double       y2         = 0.0;
    unsigned int size       = 0;

    if( segment == NULL )
    {
        return NULL;
    }

    size = 1;

    if( away_point == NULL )
    {
        size++;
    }

    result = ( LSEG ** ) palloc0( sizeof( LSEG * ) * size );

    if( result == NULL )
    {
        __oom( "Could not create output segment" );
    }

    result[0] = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( away_point == NULL )
    {
        result[1] = ( LSEG * ) palloc0( sizeof( LSEG ) );

        if( result[1] == NULL )
        {
            __oom( "Could not create orthogonal root" );
        }
    }

    if( result[0] == NULL )
    {
        __oom( "Could not create orthogonal root" );
    }

    // Store midpoint as the origin of our output vector
    result[0]->p[0].x = ( segment->p[0].x + segment->p[1].x ) / 2; // midpoint_x
    result[0]->p[0].y = ( segment->p[0].y + segment->p[1].y ) / 2; // midpoint_y

    if( away_point == NULL )
    {
        // Set midpoint for our other vector
        result[1]->p[0].x = result[0]->p[0].x;
        result[1]->p[0].y = result[0]->p[0].y;
    }

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Current slope is infinite (vertical line), target slope is 0
        result[0]->p[1].y = result[0]->p[0].y;

        if( away_point != NULL )
        {
            result[0]->p[1].x = result[0]->p[0].x + length;
            d1 = distance( away_point, (&(result[0]->p[1])) );
            result[0]->p[1].x = result[0]->p[0].x - length;
            d2 = distance( away_point, (&(result[0]->p[1])) );

            // We've stored the endpoint but if our assumtion was wrong,
            // swap the points
            if( d1 > d2 )
            {
                result[0]->p[1].x = result[0]->p[0].x + length;
            }
        }
        else
        {
            // Come up with both variants
            result[1]->p[1].y = result[0]->p[0].y;
            result[0]->p[1].x = result[0]->p[0].x + length;
            result[1]->p[1].x = result[0]->p[0].x - length;
        }
    }
    else if( fabs( segment->p[0].y - segment->p[1].y ) < DBL_EPSILON )
    {
        // Current slope is 0 (horizontal line), target slope is infinite
        result[0]->p[1].x = result[0]->p[0].x;

        if( away_point != NULL )
        {
            result[0]->p[1].y = result[0]->p[0].y + length;
            d1 = distance( away_point, (&(result[0]->p[1])) );
            result[0]->p[1].y = result[0]->p[0].y - length;
            d2 = distance( away_point, (&(result[0]->p[1])) );

            if( d1 > d2 )
            {
                result[0]->p[1].y = result[0]->p[0].y + length;
            }
        }
        else
        {
            result[1]->p[1].x = result[0]->p[0].x;
            result[0]->p[1].y = result[0]->p[0].y + length;
            result[1]->p[1].y = result[0]->p[0].y - length;
        }
    }
    else
    {
        // Solve for y = mx + b
        curr_slope = ( segment->p[0].y - segment->p[1].y ) / ( segment->p[0].x - segment->p[1].x );
        targ_slope = -1.0 / curr_slope;
        y_int      = result[0]->p[0].y - ( result[0]->p[0].x * targ_slope );

        // Find quadratic solution to sqrt( ( midpoint_x - x )^2 + ( midpoint_y - y )^2 ) = length
        // Plugging back in y = mx + b for both the unknown (y) and known (midpoint_y) values,
        // this factors to the following equation:
        // 0 = ( 1 + targ_slope^2 )( x^2 - 2 * midpoint_x * x + x_midpoint^2 ) - length^2
        // Which feeds into the following quadratic:
        a = 1.0 + ( targ_slope * targ_slope );
        b = -2.0 * a * result[0]->p[0].x;
        c = a * result[0]->p[0].x * result[0]->p[0].x - length * length;
        b2 = b * b;

        if( b2 < ( 4 * a * c ) )
        {
            __degenerate_solution( a, b, c );
            return NULL;
        }

        x1 = ( -b + sqrt( b2 - ( 4 * a * c ) ) ) / ( 2 * a );
        x2 = ( -b - sqrt( b2 - ( 4 * a * c ) ) ) / ( 2 * a );
        y1 = targ_slope * x1 + y_int;
        y2 = targ_slope * x2 + y_int;

        result[0]->p[1].x = x1;
        result[0]->p[1].y = y1;

        // Similar logic as the special case solution, find the vector end
        // farthest from away_point
        if( away_point != NULL )
        {
            d1 = distance( away_point, (&(result[0]->p[1])) );
            result[0]->p[1].x = x2;
            result[0]->p[1].y = y2;
            d2 = distance( away_point, (&(result[0]->p[1])) );

            if( d1 > d2 )
            {
                result[0]->p[1].x = x1;
                result[0]->p[1].y = y1;
            }
        }
        else
        {
            result[1]->p[1].x = x2;
            result[1]->p[1].y = y2;
        }
    }

    return result;
}

LSEG * scale_lseg( LSEG * segment, double scale_factor, Point * reference )
{
    Point  ref_point = {0};
    LSEG * result    = NULL;
    double slope     = 0.0;
    double y_int     = 0.0;
    double a         = 0.0;
    double b         = 0.0;
    double b2        = 0.0;
    double c         = 0.0;
    double length    = 0.0;

    if( segment == NULL )
    {
        return NULL;
    }

    if( fabs( scale_factor - 1.0 ) < DBL_EPSILON )
    {
        return segment;
    }

    result = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( result == NULL )
    {
        __oom( "Could not allocate result line segment" );
    }

    slope  = ( segment->p[0].y - segment->p[1].y )
           / ( segment->p[0].x - segment->p[1].x );
    y_int  = segment->p[0].y - ( slope * segment->p[0].x );
    length = sqrt(
        ( segment->p[0].x - segment->p[1].x )
      * ( segment->p[0].x - segment->p[1].x )
      + ( segment->p[0].y - segment->p[1].y )
      * ( segment->p[0].y - segment->p[1].y )
    );
    length = ( length / 2 ) * scale_factor;

    if( reference == NULL )
    {
        ref_point.x = ( segment->p[0].x + segment->p[1].x ) / 2;
        ref_point.y = ( segment->p[0].y + segment->p[1].y ) / 2;
    }
    else
    {
        ref_point.x = reference->x;
        ref_point.y = reference->y;

        if( fabs( ref_point.y - ( slope * ref_point.x + y_int ) ) >= DBL_EPSILON )
        {
            // Reference point does not lie on the line
            return NULL;
        }
    }

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Line is vertical
        result->p[0].x = segment->p[0].x;
        result->p[1].x = segment->p[1].x;
        result->p[0].y = ref_point.y - length;
        result->p[1].y = ref_point.y + length;
    }
    else if( fabs( segment->p[0].y - segment->p[1].y ) < DBL_EPSILON )
    {
        // Line is horizontal
        result->p[0].y = segment->p[0].y;
        result->p[1].y = segment->p[1].y;
        result->p[0].x = ref_point.x - length;
        result->p[1].x = ref_point.x + length;
    }
    else
    {
        a = 1.0 + ( slope * slope );
        b = -2.0 * a * ref_point.x;
        c = a * ref_point.x * ref_point.x - length * length;
        b2 = b * b;

        if( b2 < ( 4 * a * c ) )
        {
            __degenerate_solution( a, b, c );
            pfree( result );
            return NULL;
        }

        result->p[0].x = ( -b - sqrt( b2 - ( 4 * a * c ) ) ) / ( 2 * a );
        result->p[0].y = slope * result->p[0].x + y_int;
        result->p[1].x = ( -b + sqrt( b2 - ( 4 * a * c ) ) ) / ( 2 * a );
        result->p[1].y = slope * result->p[1].x + y_int;

        if( fabs( result->p[0].x ) < DBL_EPSILON )
        {
            result->p[0].x = 0.0;
        }

        if( fabs( result->p[1].x ) < DBL_EPSILON )
        {
            result->p[1].x = 0.0;
        }

        if( fabs( result->p[0].y ) < DBL_EPSILON )
        {
            result->p[0].y = 0.0;
        }

        if( fabs( result->p[1].y ) < DBL_EPSILON )
        {
            result->p[1].y = 0.0;
        }
    }

    return result;
}

// Used for extending a line segment past it's ends in one direction. The reference point
// is assumed to be one of the segments endpoints
LSEG * extend_lseg( LSEG * segment, double length, Point * endpoint )
{
    LSEG * result  = NULL;
    double slope   = 0.0;
    double y_int   = 0.0;
    double a       = 0.0;
    double b       = 0.0;
    double b2      = 0.0;
    double c       = 0.0;
    Point  other_p = {0.0};

    if( segment == NULL || endpoint == NULL )
    {
        return NULL;
    }

    if(
        !(
             (
                  fabs( endpoint->x - segment->p[0].x ) < DBL_EPSILON
               && fabs( endpoint->y - segment->p[0].y ) < DBL_EPSILON
             )
          || (
                  fabs( endpoint->x - segment->p[1].x ) < DBL_EPSILON
               && fabs( endpoint->y - segment->p[1].y ) < DBL_EPSILON
             )
         )
      )
    {
        return NULL;
    }

    result = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( result == NULL )
    {
        __oom( "Could not allocate result line segment" );
    }

    if(
            fabs( endpoint->x - segment->p[0].x ) < DBL_EPSILON
         && fabs( endpoint->y - segment->p[0].y ) < DBL_EPSILON
      )
    {
        other_p.x = segment->p[1].x;
        other_p.y = segment->p[1].y;
    }
    else
    {
        other_p.x = segment->p[0].x;
        other_p.y = segment->p[0].y;
    }

    result->p[0].x = other_p.x;
    result->p[0].y = other_p.y;

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Slope infinite
        result->p[0].x = other_p.x;

        if( other_p.y > endpoint->y )
        {
            // line is pointing down
            result->p[1].y = other_p.y - length;
        }
        else
        {
            result->p[1].y = other_p.y + length;
        }
    }
    else if( fabs( segment->p[0].y - segment->p[1].y ) < DBL_EPSILON )
    {
        // Slope is 0
        result->p[1].y = other_p.y;

        if( other_p.x > endpoint->x )
        {
            // Line is pointing left
            result->p[1].x = other_p.x - length;
        }
        else
        {
            result->p[1].x = other_p.x + length;
        }
    }
    else
    {
        slope = ( segment->p[1].y - segment->p[0].y )
              / ( segment->p[1].x - segment->p[0].x );
        y_int = segment->p[0].y - slope * segment->p[0].x;

        a = slope * slope + 1.0;
        b = 2.0 * slope * y_int - 2.0 * other_p.x - 2.0 * other_p.y * slope;
        c = other_p.y * other_p.y - 2.0 * y_int * other_p.y + y_int * y_int + other_p.x * other_p.x - length * length;
        b2 = b * b;

        if( b2  < ( 4 * a * c ) )
        {
            __degenerate_solution( a, b, c );
            pfree( result );
            return NULL;
        }

        if( slope > 0 && slope < DBL_MAX ) // Top right & bottom left quadrant
        {
            if(
                    endpoint->x > other_p.x
                 && endpoint->y > other_p.y
              ) // Top right quadrant
            {
                result->p[1].x = ( -b + sqrt( b2 - ( 4.0 * a * c ) ) ) / ( 2.0 * a );
            }
            else
            { // bottom left quadrant
                result->p[1].x = ( -b - sqrt( b2 - ( 4.0 * a * c ) ) ) / ( 2.0 * a );
            }
        }
        else
        {
            if(
                    endpoint->x < other_p.x
                 && endpoint->y > other_p.y
              ) // Top left quadrant
            {
                result->p[1].x = ( -b - sqrt( b2 - ( 4.0 * a * c ) ) ) / ( 2.0 * a );
            }
            else
            { // bottom right quadrant
                result->p[1].x = ( -b + sqrt( b2 - ( 4.0 * a * c ) ) ) / ( 2.0 * a );
            }
        }

        result->p[1].y = slope * result->p[1].x + y_int;
    }

    return result;
}

// Note: segment_b here is the reference line, the angle given is based off
// of its orientation.
double get_angle_of_lseg_intersection( LSEG * segment_a, LSEG * segment_b )
{
    double slope_a = 0.0;
    double slope_b = 0.0;
    double result  = 0.0;

    if( segment_a == NULL || segment_b == NULL )
    {
        return 0.0;
    }

    slope_a = ( segment_a->p[0].y - segment_a->p[1].y )
            / ( segment_a->p[0].x - segment_a->p[1].x );
    slope_b = ( segment_b->p[0].y - segment_b->p[1].y )
            / ( segment_b->p[0].x - segment_b->p[1].x );
    result  = atan( slope_a ) - atan( slope_b );

    return result;
}

Point * lseg_to_vector( LSEG * segment )
{
    Point * result = NULL;
    double  slope  = 0.0;
    //double  y_int  = 0.0;
    double  length = 0.0;
    double  l2     = 0.0;
    double  s2     = 0.0;

    if( segment == NULL )
    {
        return NULL;
    }

    result = ( Point * ) palloc0( sizeof( Point ) );

    if( result == NULL )
    {
        __oom( "Could no allocate result line segment" );
    }

    length = sqrt(
        ( segment->p[0].x - segment->p[1].x )
      * ( segment->p[0].x - segment->p[1].x )
      + ( segment->p[0].y - segment->p[1].y )
      * ( segment->p[0].y - segment->p[1].y )
    );

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Line is vertical
        result->x = 0;

        if( segment->p[1].y < segment->p[0].y )
        {
            result->y = -length;
        }
        else
        {
            result->y = length;
        }

        return result;
    }
    else if( fabs( segment->p[1].y - segment->p[0].y ) < DBL_EPSILON )
    {
        result->y = 0;

        if( segment->p[1].x < segment->p[0].x )
        {
            result->x = -length;
        }
        else
        {
            result->x = length;
        }

        return result;
    }

    slope = ( segment->p[1].y - segment->p[0].y )
          / ( segment->p[1].x - segment->p[0].x );
    //y_int = segment->p[0].y - slope * segment->p[0].x;

    l2 = length * length;
    s2 = slope * slope;

    if( segment->p[1].x > segment->p[0].x )
    {
        result->x = sqrt( l2 / ( 1.0 + s2 ) );
    }
    else
    {
        result->x = -sqrt( l2 / ( 1.0 + s2 ) );
    }

    result->y = result->x * slope;

    return result;
}

// For these two functions, the order of the points implies the lseg's direction
// component
bool lseg_points_right_of( LSEG * segment, LSEG * reference )
{
    Point * seg_vector = NULL;
    Point * ref_vector = NULL;

    if( segment == NULL || reference == NULL )
    {
        return false;
    }

    seg_vector = lseg_to_vector( segment );
    ref_vector = lseg_to_vector( reference );

    if( seg_vector == NULL || ref_vector == NULL )
    {
        if( seg_vector != NULL )
        {
            pfree( seg_vector );
        }

        if( ref_vector != NULL )
        {
            pfree( ref_vector );
        }

        return false;
    }

    if( cross_product( seg_vector, ref_vector ) > 0 )
    {
        return true;
    }

    return false;
}

bool lseg_points_left_of( LSEG * segment, LSEG * reference )
{
    Point * seg_vector = NULL;
    Point * ref_vector = NULL;

    if( segment == NULL || reference == NULL )
    {
        return false;
    }

    seg_vector = lseg_to_vector( segment );
    ref_vector = lseg_to_vector( reference );

    if( seg_vector == NULL || ref_vector == NULL )
    {
        if( seg_vector != NULL )
        {
            pfree( seg_vector );
        }

        if( ref_vector != NULL )
        {
            pfree( ref_vector );
        }

        return false;
    }

    return cross_product( seg_vector, ref_vector ) < 0;
}

bool point_is_right_of_with_ref( LSEG * segment, Point * point, Point * front_ref )
{
    Point  A   = {0.0};
    Point  B   = {0.0};
    double det = 0.0;

    if( segment == NULL || point == NULL || front_ref == NULL )
    {
        return false;
    }

    A.x = segment->p[0].x;
    A.y = segment->p[0].y;
    B.x = segment->p[1].x;
    B.y = segment->p[1].y;

    if(
        sqrt(
            ( A.x - front_ref->x ) * ( A.x - front_ref->x )
          + ( A.y - front_ref->y ) * ( A.y - front_ref->y )
        ) >
        sqrt(
            ( B.x - front_ref->x ) * ( B.x - front_ref->x )
          + ( B.y - front_ref->y ) * ( B.y - front_ref->y )
        )
      )
    {
        A.x = segment->p[1].x;
        A.y = segment->p[1].y;
        B.x = segment->p[0].x;
        B.y = segment->p[0].y;
    }

    det = ( B.x - A.x ) * ( point->y - A.y ) - ( B.y - A.y ) * ( point->x - A.x );

    return ( fabs( det ) >= DBL_EPSILON ) && ( ( det - DBL_EPSILON ) <= 0 );
}

bool point_is_left_of_with_ref( LSEG * segment, Point * point, Point * front_ref )
{
    Point  A   = {0.0};
    Point  B   = {0.0};
    double det = 0.0;

    if( segment == NULL || point == NULL || front_ref == NULL )
    {
        return false;
    }

    A.x = segment->p[0].x;
    A.y = segment->p[0].y;
    B.x = segment->p[1].x;
    B.y = segment->p[1].y;

    if(
        sqrt(
            ( A.x - front_ref->x ) * ( A.x - front_ref->x )
          + ( A.y - front_ref->y ) * ( A.y - front_ref->y )
        ) >
        sqrt(
            ( B.x - front_ref->x ) * ( B.x - front_ref->x )
          + ( B.y - front_ref->y ) * ( B.y - front_ref->y )
        )
      )
    {
        A.x = segment->p[1].x;
        A.y = segment->p[1].y;
        B.x = segment->p[0].x;
        B.y = segment->p[0].y;
    }

    det = ( B.x - A.x ) * ( point->y - A.y ) - ( B.y - A.y ) * ( point->x - A.x );

    if( fabs( det ) < DBL_EPSILON )
    {
        return false;
    }

    if( ( det - DBL_EPSILON ) > 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

LSEG * get_root_orthogonal_segment( LSEG * segment, Point * endpoint, double length )
{
    /*
     *  Returns a segment orthogonal to the input at the endpoint (the endpoint forms the midpoint of the output)
     *  The result is of the specified length
     */
    LSEG * result     = NULL;
    double curr_slope = 0.0;
    double targ_slope = 0.0;
    double y_int      = 0.0;
    double a          = 0.0;
    double b          = 0.0;
    double b2         = 0.0;
    double c          = 0.0;
    double half_l     = 0.0;

    if( segment == NULL || endpoint == NULL )
    {
        return NULL;
    }

    if(
           !(
               fabs( endpoint->x - segment->p[0].x ) < DBL_EPSILON
            && fabs( endpoint->y - segment->p[0].y ) < DBL_EPSILON
           )
        && !(
               fabs( endpoint->x - segment->p[1].x ) < DBL_EPSILON
            && fabs( endpoint->y - segment->p[1].y ) < DBL_EPSILON
           )
      )
    {
        // Endpoint is not an endpoint of the input line segment
        elog( DEBUG1, "Specified endpoint is not an endpoint on the input line segment!" );
        return NULL;
    }

    result = ( LSEG * ) palloc0( sizeof( LSEG ) );
    if( result == NULL )
    {
        __oom( "Could not allocate result line segment" );
    }

    half_l = length / 2.0;

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Current slope is infinite, target slope is 0
        result->p[0].y = endpoint->y;
        result->p[1].y = endpoint->y;
        result->p[0].x = endpoint->x - half_l;
        result->p[1].x = endpoint->x + half_l;
    }
    else if( fabs( segment->p[0].y - segment->p[1].y ) < DBL_EPSILON )
    {
        result->p[0].x = endpoint->x;
        result->p[1].x = endpoint->x;
        result->p[0].y = endpoint->y - half_l;
        result->p[1].y = endpoint->y + half_l;
    }
    else
    {
        curr_slope = ( segment->p[0].y - segment->p[1].y )
                   / ( segment->p[0].x - segment->p[1].x );
        targ_slope = -1.0 / curr_slope;
        y_int      = endpoint->y - ( endpoint->x * targ_slope );
        a          = 1.0 + ( targ_slope * targ_slope );
        b          = -2.0 * a * endpoint->x;
        c          = a * endpoint->x * endpoint->x - ( half_l * half_l );
        b2         = b * b;

        if( b2 < ( 4 * a * c ) )
        {
            __degenerate_solution( a, b, c );
            pfree( result );
            return NULL;
        }

        result->p[0].x = ( -b + sqrt( b2 - ( 4.0 * a * c ) ) ) / ( 2.0 * a );
        result->p[0].y = targ_slope * result->p[0].x + y_int;
        result->p[1].x = ( -b - sqrt( b2 - ( 4.0 * a * c ) ) ) / ( 2.0 * a );
        result->p[1].y = targ_slope * result->p[1].x + y_int;
    }
    return result;
}
