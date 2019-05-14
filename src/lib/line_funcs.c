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
        __oom( "Failed to allocate line array" );
    }

    result[0] = ( LINE * ) palloc0( sizeof( LINE ) );

    if( result[0] == NULL )
    {
        __oom( "Failed to allocate line array element" );
    }

    if( size > 1 )
    {
        result[1] = ( LINE * ) palloc0( sizeof( LINE ) );

        if( result[1] == NULL )
        {
             __oom( "Failed to allocate line array element" );
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
        __oom( "Failed to allocat ereturn line for orthogonal calculation" );
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

LINE * polygon_to_line( POLYGON * p )
{
    LINE * result = NULL;

    if( p == NULL )
    {
        return NULL;
    }

    result = best_fit_line( p->p, p->npts );

    return result;
}

LINE * point_to_line( Point * p )
{
    LINE * result = NULL;

    if( p == NULL )
    {
        return NULL;
    }

    result = ( LINE * ) palloc0( sizeof( LINE ) );
    
    if( result == NULL )
    {
        __oom( "Failed to allocate line for point cast" );
    }

    result->B = -1.0;
    result->A = ( p->y / p->x );
    result->C = 0.0; 

    return result;
}

LINE * lseg_to_line( LSEG * l )
{
    LINE * result = NULL;

    if( l == NULL )
    {
        return NULL;
    }

    result = ( LINE * ) palloc0( sizeof( LINE ) );

    if( result == NULL )
    {
        __oom( "Failed to allocate line for lseg cast" );
    }

    result->B = -1.0;
    result->A = ( l->p[1].y - l->p[0].y )
              / ( l->p[1].x - l->p[0].x );
    result->C = l->p[0].y - ( result->A / result->B ) * l->p[0].x;

    return result;
}

LINE * path_to_line( PATH * p )
{
    LINE * result = NULL;

    if( p == NULL )
    {
        return NULL;
    }

    result = best_fit_line( p->p, p->npts );

    return result;
}

LINE * box_to_line( BOX * b )
{
    LINE * result = NULL;
    
    if( b == NULL )
    {
        return NULL;
    }

    result = ( LINE * ) palloc0( sizeof( LINE ) );

    if( result == NULL )
    {
        __oom( "Failed to allocate line for box cast" );
    }

    result->B = -1.0;
    result->A = ( b->high.y - b->low.y )
              / ( b->high.x - b->low.x );
    result->C = b->low.y - ( result->A / result->B ) * b->low.x;

    return result;
}

// We cannot preserve much information from the circle to encode within the line
// We will generate a line with slope = radius, 
LINE * circle_to_line( CIRCLE * c )
{
    LINE * result = NULL;

    if( c == NULL )
    {
        return NULL;
    }

    result = ( LINE * ) palloc0( sizeof( LINE ) );

    if( result == NULL )
    {
        __oom( "Failed to allocate line for circle cast" );
    }

    result->B = -1.0; // maybe we can store the x coordinate of the radius here,
                      // but that would distort the line
    result->A = c->radius;
    result->C = c->center.y;

    return result;
}
