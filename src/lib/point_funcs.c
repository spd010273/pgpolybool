/*------------------------------------------------------------------------------
 * point_funcs.c
 *      Helper functions of pgpolybool point functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      point_funcs.c
 *
 *------------------------------------------------------------------------------
 */

#include "point_funcs.h"

Point * line_to_point( LINE * line )
{
    Point * result = NULL;

    if( line == NULL )
    {
        return NULL;
    }

    result = ( Point * ) palloc0( sizeof( Point ) );

    if( result == NULL )
    {
        __oom( "Failed to allocate point for line cast" );
    }

    result->y = -1.0 * ( line->C / line->B );
    result->x = 0.0;

    return result;
}

Point * path_to_point( PATH * path )
{
    Point * result = NULL;
    unsigned int i = 0;

    if( path == NULL )
    {
        return NULL;
    }

    result = ( Point * ) palloc0( sizeof( Point ) );

    if( result == NULL )
    {
        __oom( "Failed to allocate point for path cast" );
    }

    result->x = 0.0;
    result->y = 0.0;

    for( i = 0; i < path->npts; i++ )
    {
        result->x += path->p[i].x;
        result->y += path->p[i].y;
    }

    result->x = result->x / path->npts;
    result->y = result->y / path->npts;

    return result;
}

// Takes in an array of points
LINE * best_fit_line( Point * p, unsigned int len )
{
    LINE *       result     = NULL;
    unsigned int i          = 0;
    double       x_avg      = 0.0;
    double       y_avg      = 0.0;
    double       n_delta    = 0.0;
    double       d_delta    = 0.0;
    double       slope      = 0.0;
    double       y_int      = 0.0;

    if( p == NULL )
    {
        return NULL;
    }

    result = ( LINE * ) palloc0( sizeof( LINE ) );

    if( result == NULL )
    {
        __oom( "Failed to allocate line for best-fit line calculation" );
    }

    for( i = 0; i < len; i++ )
    {
        x_avg += p[i].x;
        y_avg += p[i].y;
    }

    x_avg = x_avg / len;
    y_avg = y_avg / len;

    for( i = 0; i < len; i++ )
    {
        n_delta = ( p[i].x - x_avg ) * ( p[i].y - y_avg );
        d_delta = pow( ( p[i].x - x_avg ), 2 );
    }

    slope = n_delta / d_delta;
    y_int = y_avg - slope * x_avg;

    result->A = -slope;
    result->B = 1.0;
    result->C = -y_int;

    return result;
}
