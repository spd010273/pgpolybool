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
        return NULL;
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
        return NULL;
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
