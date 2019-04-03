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
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create output segment" )
            )
        );
    }

    result[0] = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( away_point == NULL )
    {
        result[1] = ( LSEG * ) palloc0( sizeof( LSEG ) );

        if( result[1] == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not create orthogonal root" )
                )
            );
        }
    }

    if( result[0] == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create orthogonal root" )
            )
        );
    }

    if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Current slope is infinite (vertical line)
        result[0]->p[0].y = segment->p[0].y;
        result[0]->p[1].y = segment->p[1].y;

        if( away_point != NULL )
        {
            result[0]->p[0].x = segment->p[0].x + line_distance;
            d1 = distance( away_point, &(result[0]->p[0]) );
            result[0]->p[0].x = segment->p[0].x - line_distance;
            d2 = distance( away_point, &(result[0]->p[0]) );

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
            d1 = distance( away_point, &(result[0]->p[0]) );
            result[0]->p[0].y = segment->p[0].y - line_distance;
            d2 = distance( away_point, &(result[0]->p[0]) );

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
            elog( DEBUG1, "solution for quadratic a=%f, b=%f, c=%f is degenerate", a, b_0, c_0 );

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
            elog( DEBUG1, "solution for quadratic a=%f, b=%f, c=%f is degenerate", a, b_1, c_1 );
            pfree( result[0] );

            if( away_point == NULL )
            {
                pfree( result[1] );
            }

            pfree( result );
            return NULL;
        }

        // coordinate<solution #>_<point_index>

        x1_0 = ( -b_0 + sqrt( pow( b_0, 2 ) - ( 4 * a * c_0 ) ) ) / ( 2 * a );
        y1_0 = targ_slope * x1_0 + y_int_0;

        x2_0 = ( -b_0 - sqrt( pow( b_0, 2 ) - ( 4 * a * c_0 ) ) ) / ( 2 * a );
        y2_0 = targ_slope * x2_0 + y_int_0;

        x1_1 = ( -b_1 + sqrt( pow( b_1, 2 ) - ( 4 * a * c_1 ) ) ) / ( 2 * a );
        y1_1 = targ_slope * x1_1 + y_int_1;

        x2_1 = ( -b_1 - sqrt( pow( b_1, 2 ) - ( 4 * a * c_1 ) ) ) / ( 2 * a );
        y2_1 = targ_slope * x2_1 + y_int_1;

        result[0]->p[0].x = x1_0;
        result[0]->p[0].y = y1_0;
        result[0]->p[1].x = x1_1;
        result[0]->p[1].y = y1_1;

        // Similar logic as the special case solution, find the vector end
        // farthest from away_point
        if( away_point != NULL )
        {
            d1 = distance( away_point, &(result[0]->p[0]) );
            result[0]->p[0].x = x2_0;
            result[0]->p[0].y = y2_0;
            d2 = distance( away_point, &(result[0]->p[0]) );

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
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create output segment" )
            )
        );
    }

    result[0] = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( away_point == NULL )
    {
        result[1] = ( LSEG * ) palloc0( sizeof( LSEG ) );

        if( result[1] == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not create orthogonal root" )
                )
            );
        }
    }

    if( result[0] == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create orthogonal root" )
            )
        );
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
            d1 = distance( away_point, &(result[0]->p[1]) );
            result[0]->p[1].x = result[0]->p[0].x - length;
            d2 = distance( away_point, &(result[0]->p[1]) );

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
            d1 = distance( away_point, &(result[0]->p[1]) );
            result[0]->p[1].y = result[0]->p[0].y - length;
            d2 = distance( away_point, &(result[0]->p[1]) );

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

        if( pow( b, 2 ) < ( 4 * a * c ) )
        {
            elog( DEBUG1, "solution for quadratic a=%f, b=%f, c=%f is degenerate", a, b, c );
            return NULL;
        }

        x1 = ( -b + sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
        x2 = ( -b - sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
        y1 = targ_slope * x1 + y_int;
        y2 = targ_slope * x2 + y_int;

        result[0]->p[1].x = x1;
        result[0]->p[1].y = y1;

        // Similar logic as the special case solution, find the vector end
        // farthest from away_point
        if( away_point != NULL )
        {
            d1 = distance( away_point, &(result[0]->p[1]) );
            result[0]->p[1].x = x2;
            result[0]->p[1].y = y2;
            d2 = distance( away_point, &(result[0]->p[1]) );

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
        return NULL;
    }

    slope  = ( segment->p[0].y - segment->p[1].y )
           / ( segment->p[0].x - segment->p[1].x );
    y_int  = segment->p[0].y - ( slope * segment->p[0].x );
    length = sqrt(
        pow( segment->p[0].x - segment->p[1].x, 2 )
      + pow( segment->p[0].y - segment->p[1].y, 2 )
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

        if( pow( b, 2 ) < ( 4 * a * c ) )
        {
            pfree( result );
        }

        result->p[0].x = ( -b - sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
        result->p[0].y = slope * result->p[0].x + y_int;
        result->p[1].x = ( -b + sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
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

// Note: segment_b here is the reference line, the angle given is based off
// of its orientation.
double get_angle_of_intersection( LSEG * segment_a, LSEG * segment_b )
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

    if( segment == NULL )
    {
        return NULL;
    }

    result = ( Point * ) palloc0( sizeof( Point ) );

    if( result == NULL )
    {
        return NULL;
    }

    length = sqrt(
        pow( segment->p[0].x - segment->p[1].x, 2 )
      + pow( segment->p[0].y - segment->p[1].y, 2 )
    );

    if( abs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
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
    else if( abs( segment->p[1].y - segment->p[0].y ) < DBL_EPSILON )
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

    if( segment->p[1].x > segment->p[0].x )
    {
        result->x = sqrt( pow( length, 2 ) / ( 1 + pow( slope, 2 ) ) );
    }
    else
    {
        result->x = -sqrt( pow( length, 2 ) / ( 1 + pow( slope, 2 ) ) );
    }

    result->y = result->x * slope;

    return result;
}

double cross_product( Point * p1, Point * p2 )
{
    if( p1 == NULL || p2 == NULL )
    {
        return 0.0;
    }

    return ( p1->x * p2->y - p1->y * p2->x );
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

    if( cross_product( seg_vector, ref_vector ) < 0 )
    {
        return true;
    }

    return false;
}
