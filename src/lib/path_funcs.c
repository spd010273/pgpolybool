/*------------------------------------------------------------------------------
 * path_funcs.c
 *     pgpolybool path functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      path_funcs.c
 *
 *------------------------------------------------------------------------------
 */

#include "path_funcs.h"

Point ** get_path_points( PATH * p )
{
    Point **     result = NULL;
    unsigned int i      = 0;
    unsigned int j      = 0;

    if( p == NULL || p->npts == 0 )
    {
        return NULL;
    }

    result = ( Point ** ) palloc0( sizeof( Point * ) * p->npts );

    if( result == NULL )
    {
        return NULL;
    }

    for( i = 0; i < p->npts; i++ )
    {
        result[i] = ( Point * ) palloc0( sizeof( Point ) );

        if( result[i] == NULL )
        {
            for( j = 0; j < i; j++ )
            {
                pfree( result[j] );
            }

            pfree( result );
            return NULL;
        }

        result[i]->x = p->p[i].x;
        result[i]->y = p->p[i].y;
   }

    return result;
}

LSEG ** get_path_lsegs( PATH * p )
{
    LSEG ** result = NULL;


    return result;
}
