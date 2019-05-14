/*------------------------------------------------------------------------------
 * line_funcs.c
 *      Helper functions of pgpolybool line functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      line_funcs.c
 *
 *------------------------------------------------------------------------------
 */

#include "line_funcs.h"

LINE ** line_parallel_line( LINE * line, Point * away_point, double distance )
{
    LINE **      result = NULL;
    double       slope  = 0.0;
    double       y_int  = 0.0;
    double       dy_int = 0.0;
    unsigned int size   = 0;

    if( line == NULL )
    {
        return NULL;
    }

    size = 1;

    if( away_point == NULL )
    {
        size++;
    }

    result = ( LINE ** ) palloc0( sizeof( LINE * ) * size );

    if( result == NULL )
    {
        return NULL;
    }

    result[0] = ( LINE * ) palloc0( sizeof( LINE ) );

    if( result[0] == NULL )
    {
        return NULL;
    }

    if( size > 1 )
    {
        result[1] = ( LINE * ) palloc0( sizeof( LINE ) );

        if( result[1] == NULL )
        {
            return NULL;
        }
    }

    slope  = -( line->A / line->B );
    y_int  = -( line->C / line->B );
    dy_int = distance / cos( atan( slope ) );

    result[0]->B = -1.0;
    result[0]->A = slope;

    if( away_point == NULL )
    {
        result[0]->C = y_int + dy_int;

        result[1]->B = -1.0;
        result[1]->A = slope;
        result[1]->C = y_int - dy_int;
    }
    else
    {
        if( fabs( y_int + dy_int - away_point->y ) > fabs( ( y_int - dy_int ) - away_point->y ) )
        {
            result[0]->C = y_int + dy_int;
        }
        else
        {
            result[0]->C = y_int - dy_int;
        }
    }

    return result;
}

// Return a line orthogonal to input. If reference point is provided, the result
// will pass through that point
LINE * line_orthogonal_line( LINE * line, Point * reference_point )
{
    LINE * result = NULL;

    if( line == NULL )
    {
        return NULL;
    }

    result = ( LINE * ) palloc0( sizeof( LINE ) );
    
    if( result == NULL )
    {
        return NULL;
    }

    result->B = -1.0;
    result->A = line->B / line->A;

    if( reference_point == NULL )
    {
        result->C = 0.0;
    }
    else
    {
        result->C = reference_point->y + result->A * reference_point->x;
    }

    return result;
}

double get_angle_of_line_intersection( LINE * l1, LINE * l2 )
{
    double result = 0.0;

    if( l1 == NULL || l2 == NULL )
    {
        return NAN;
    }

    if( fabs( ( l1->A / l1->B ) - ( l2->A / l2->B ) ) < DBL_EPSILON )
    {
        // Parallel
        return NAN;
    }

    result = PI - fabs( atan( -l1->A / l1->B ) - atan( -l2->A / l2->B ) );

    return result;
}
