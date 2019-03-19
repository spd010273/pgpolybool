/*------------------------------------------------------------------------------
 * conv.c
 *      Helper functions of pgpolybool geometric type conversions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      conv.c
 *
 *------------------------------------------------------------------------------
 */

#include "conv.h"

/* conversion listing:
 * box->polygon: reimplemented here
 * polygon->box: PostgreSQL native
 * box->circle:  PostgreSQL native
 * circle->box:  PostgreSQL native
 * path->box:    implemented here
 * box->path:    implemented here
 */
POLYGON * box_to_polygon( BOX * b )
{
    Point **     b_points = NULL;
    POLYGON *    result   = NULL;
    unsigned int i        = 0;

    if( b == NULL )
    {
        return NULL;
    }
    
    b_points = get_box_points( b );

    if( b_points == NULL )
    {
        return NULL;
    }
    
    result = ( POLYGON * ) palloc0(
        offsetof( POLYGON, p )
      + ( sizeof( Point ) * 4 )
    );

    if( result == NULL )
    {
        for( i = 0; i < 4; i++ )
        {
            pfree( b_points[i] );
        }

        pfree( b_points );
        return NULL;
    }
    
    for( i = 0; i < 4; i++ )
    {
        result->p[i].x = b_points[i]->x;
        result->p[i].y = b_points[i]->y;
    }
    
    result->npts = 4;

    set_polygon_boundbox( result );
    return result;
}
