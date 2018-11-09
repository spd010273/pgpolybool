/*------------------------------------------------------------------------------
 * polyprocessing.c
 *     Polygon frontend and backend processing functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      polyprocessing.c
 *
 *------------------------------------------------------------------------------
 */

#include "polyprocessing.h"
#include "util.h"

/*
 * Preprocessing frontend that handles buffering, scaling and sorting of polys
 * from function input
 * We CANNOT modify the original polygons, as they may be pointers to something
 * in the disk buffer.
 */

POLYGON * poly_preprocessing(
    POLYGON * current_poly,
    bool scale,
    Point ** center
)
{
    POLYGON * buff_poly    = NULL;
    unsigned int npoints   = 0;
    unsigned int palloc_sz = 0;
    unsigned int i         = 0;
    double sum_x           = 0;
    double sum_y           = 0;

    palloc_sz = offsetof( POLYGON, p )
              + sizeof( current_poly->p[0] )
              * current_poly->npts;
    buff_poly = ( POLYGON * ) palloc0( palloc_sz );
    (*center) = ( Point * ) palloc0( palloc_sz );

    if( buff_poly == NULL || (*center) == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg(
                    "Could not allocate centroid or buffer for polygon"
                )
            )
        );
    }

    for( i = 0; i < current_poly->npts; i++ )
    {
        sum_x += current_poly->p[i].x;
        sum_y += current_poly->p[i].y;
        npoints++;
    }

    (*center)->x = sum_x / current_poly->npts;
    (*center)->y = sum_y / current_poly->npts;

    for( i = 0; i < current_poly->npts; i++ )
    {
        if( scale )
        {
            buff_poly->p[i].x = current_poly->p[i].x * ZOOM_RATE
                              - ( ZOOM_RATE - 1 ) * (*center)->x;
            buff_poly->p[i].y = current_poly->p[i].y * ZOOM_RATE
                              - ( ZOOM_RATE - 1 ) * (*center)->y;
        }
        else
        {
            buff_poly->p[i].x = current_poly->p[i].x;
            buff_poly->p[i].y = current_poly->p[i].y;
        }
    }

    buff_poly->npts     = current_poly->npts;
    buff_poly->boundbox = current_poly->boundbox;

    return buff_poly;
}

POLYGON ** poly_preprocessing_array(
    ArrayType * polyarray,
    bool sort,
    bool scale,
    Point *** centers,
    unsigned int * num_poly
)
{
    Datum *      dpoly             = NULL;
    Point *      center            = NULL;
    POLYGON **   ret               = NULL;
    POLYGON **   buff_polys        = NULL;
    POLYGON **   buff_polys_sorted = NULL;
    POLYGON *    buff_poly         = NULL;
    POLYGON *    current_poly      = NULL;
    bool *       nulls             = NULL;
    double       last_dist         = 0.0;
    double       min_dist          = 0.0;
    unsigned int i                 = 0;
    unsigned int palloc_sz         = 0;
    unsigned int remaining         = 0;
    unsigned int last_dist_ind     = 0;
    unsigned int sort_ind          = 0;

    // Values for POLYGON type taken from pg_catalog.pg_type
    deconstruct_array(
        polyarray,
        POLYGONOID,
        -1,
        false,
        'd',
        &dpoly,
        &nulls,
        (int *) num_poly
    );

    if( (*num_poly) <= 1 )
    {
        buff_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

        if( buff_polys == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not buffer input polygons" )
                )
            );
        }

        buff_poly     = DatumGetPolygonP( dpoly[0] );
        buff_polys[0] = buff_poly;

        return buff_polys;
    }

    for( i = 0; i < (*num_poly); i++ )
    {
        if( nulls[i] )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_NULL_VALUE_NOT_ALLOWED ),
                    errmsg( "polygon array may not contain nulls" )
                )
            );
        }
    }

    // Allocate structs
    palloc_sz  = sizeof( POLYGON * ) * (*num_poly);
    buff_polys = ( POLYGON ** ) palloc0( palloc_sz );

    if( buff_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not buffer input polygons" )
            )
        );
    }

    palloc_sz  = (*num_poly) * sizeof( Point * );
    (*centers) = ( Point ** ) palloc0( palloc_sz );

    if( (*centers) == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not allocate polygon centroids" )
            )
        );
    }

    for( i = 0; i < (*num_poly); i++ )
    {
        current_poly  = DatumGetPolygonP( dpoly[i] );
        buff_poly     = poly_preprocessing( current_poly, scale, &center );
        buff_polys[i] = buff_poly;
        (*centers)[i] = center;
    }

    if( sort )
    {
        buff_polys_sorted = ( POLYGON ** ) palloc0(
            sizeof( POLYGON * ) * (*num_poly)
        );

        if( buff_polys_sorted == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not create sorting array for polygons" )
                )
            );
        }

        // Sort polys by center-center distance from buff_poly[0]
        buff_polys_sorted[0] = buff_polys[0];
        buff_polys[0]        = NULL;
        remaining            = (*num_poly) - 1;
        sort_ind             = 1;

        while( remaining > 0 )
        {
            min_dist = DBL_MAX;

            for( i = 1; i < (*num_poly); i++ )
            {
                if( buff_polys[i] != NULL )
                {
                    last_dist = distance( (*centers)[0], (*centers)[i] );

                    if( last_dist < min_dist )
                    {
                        min_dist = last_dist;
                        last_dist_ind = i;
                    }
                }
            }

            buff_polys_sorted[sort_ind] = buff_polys[last_dist_ind];
            buff_polys[last_dist_ind]   = NULL;
            remaining--;
            sort_ind++;
        }

        pfree( buff_polys );
    }
    else
    {
        buff_polys_sorted = buff_polys;
    }

    if( buff_polys_sorted != NULL )
    {
        ret = buff_polys_sorted;
    }

    return ret;
}

POLYGON * poly_postprocessing(
    POLYGON *    poly,
    Point **     centers,
    unsigned int num_centers,
    bool         scale
)
{
    Point *      center   = NULL;
    double       sum_x    = 0.0;
    double       sum_y    = 0.0;
    double       distance = 0.0
    unsigned int i        = 0;
    unsigned int j        = 0;

    if( poly == NULL )
    {
        return NULL;
    }

    if( scale )
    {
        // De-scale the poly by ZOOM_RATE
        center = ( Point * ) palloc0( sizeof( Point ) );

        if( center == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not allocate output polygon centroid" )
                )
            );
        }

        for( i = 0; i < poly->npts; i++ )
        {
            sum_x += poly->p[i].x;
            sum_y += poly->p[i].y;
        }

        center->x = sum_x / poly->npts;
        center->y = sum_y / poly->npts;

        for( i = 0; i < poly->npts; i++ )
        {
            poly->p[i].x = ( poly->p[i].x + ( ZOOM_RATE - 1 ) * center->x )
                         / ZOOM_RATE;
            poly->p[i].y = ( poly->p[i].y + ( ZOOM_RATE - 1 ) * center->y )
                         / ZOOM_RATE;
        }

        pfree( center );

        /*
        for( i = 0; i < num_centers; i++ )
        {
            center = centers[i];

            for( j = 0; j < poly->npts; j++ )
            {
                distance = distance( center, poly->p[j] );
            }
        }
        */
    }

    // TODO: Eliminate duplicate points and points that are coincident / colinear
    return poly;
}

#ifdef DEBUG
static void dump_polygon( POLYGON * p )
{
    unsigned int i = 0;

    elog( DEBUG1,
        "\nDumping POLYGON======================== ADDR: %9p\n"\

        "HEADER: %d\n"\
        "npts: %d\n"\
        "BOX HI: (%f,%f)\n"\
        "BOX LO: (%f,%f)\n"\
        "POINTS:",
        ( void * ) p,
        p->vl_len_,
        p->npts,
        p->boundbox.high.x,
        p->boundbox.high.y,
        p->boundbox.low.x,
        p->boundbox.low.y
    );

    for( i = 0; i < p->npts; i++ )
    {
        elog( DEBUG1, "Point[%d]: (%f,%f)", i, p->p[i].x, p->p[i].y );
    }

    elog( DEBUG1, "=======================================================" );

    return;
}
#endif // DEBUG
