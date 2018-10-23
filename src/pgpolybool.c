#include "postgres.h"
#include "utils/array.h"
#include "utils/geo_decls.h"
#include "catalog/pg_type.h"
#include "fmgr.h"

#include "martinez.h"

#define ZOOM_RATE 1.04

// Helper Routines
POLYGON ** poly_preprocessing( ArrayType *, bool, bool, int * );
POLYGON * poly_postprocessing( POLYGON *, bool );

// Debug Routines
#ifdef DEBUG
static void dump_polygon( POLYGON * );
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(fn_union_polygons);
PG_FUNCTION_INFO_V1(fn_intersect_polygons);
PG_FUNCTION_INFO_V1(fn_subtract_polygons);

Datum fn_subtract_polygons( PG_FUNCTION_ARGS )
{
    int        num_poly     = 0;
    int        i            = 0;
    POLYGON ** sorted_polys = NULL;
    POLYGON *  new_polygon  = NULL;
    struct polygon * mp_subj = NULL;
    struct polygon * mp_clip = NULL;
    struct polygon * mp_result = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing(
        PG_GETARG_ARRAYTYPE_P(0),
        false,
        false,
        &num_poly
    );

    if( sorted_polys == NULL )
    {
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
    }

    if( num_poly == 1 )
    {
        elog( DEBUG1, "Only 1 poly passed :|" );
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( sorted_polys[0] );
        dump_polygon( sorted_polys[i] );
#endif
        mp_subj = poly_to_mpoly( sorted_polys[0] );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        compute(
            mp_subj,
            mp_clip,
            OP_DIFFERENCE,
            mp_result
        );

        new_polygon = mpoly_to_poly( mp_result );

        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
            PG_RETURN_NULL();
        }

        pfree( sorted_polys[0] );
        sorted_polys[0] = new_polygon;
    }

    for( i = 0; i < num_poly; i++ )
    {
        pfree( sorted_polys[i] );
    }

    pfree( sorted_polys );

    if( new_polygon == NULL )
    {
        PG_RETURN_NULL();
    }

#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_intersect_polygons( PG_FUNCTION_ARGS )
{
    int        num_poly     = 0;
    int        i            = 0;
    POLYGON ** sorted_polys = NULL;
    POLYGON *  new_polygon  = NULL;
    struct polygon * mp_subj = NULL;
    struct polygon * mp_clip = NULL;
    struct polygon * mp_result = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing(
        PG_GETARG_ARRAYTYPE_P(0),
        true,
        false,
        &num_poly
    );

    if( num_poly == 1 )
    {
        elog( DEBUG1, "Only 1 poly passed :|" );
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    if( sorted_polys == NULL )
    {
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
    }

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( sorted_polys[0] );
        dump_polygon( sorted_polys[i] );
#endif
        mp_subj = poly_to_mpoly( sorted_polys[0] );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        compute(
            mp_subj,
            mp_clip,
            OP_INTERSECTION,
            mp_result
        );

        new_polygon = mpoly_to_poly( mp_result );

        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
            PG_RETURN_NULL();
        }

        pfree( sorted_polys[0] );
        sorted_polys[0] = new_polygon;
    }

    for( i = 0; i < num_poly; i++ )
    {
        pfree( sorted_polys[i] );
    }

    pfree( sorted_polys );

    if( new_polygon == NULL )
    {
        PG_RETURN_NULL();
    }

#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_union_polygons( PG_FUNCTION_ARGS )
{
    int        num_poly        = 0;
    int        i               = 0;
    POLYGON ** sorted_polys    = NULL;
    POLYGON *  new_polygon     = NULL;
    struct polygon * mp_subj   = NULL;
    struct polygon * mp_clip   = NULL;
    struct polygon * mp_result = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing(
        PG_GETARG_ARRAYTYPE_P(0),
        true,
        true,
        &num_poly
    );

    if( sorted_polys == NULL )
    {
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
    }

    if( num_poly == 1 )
    {
        elog( DEBUG1, "Only 1 poly passed :|" );
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( sorted_polys[0] );
        dump_polygon( sorted_polys[i] );
#endif
        mp_subj = poly_to_mpoly( sorted_polys[0] );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        elog( DEBUG1, "Entering compute with OP_UNION" );
        compute(
            mp_subj,
            mp_clip,
            OP_UNION,
            mp_result
        );
        elog( DEBUG1, "Compute with OP_UNION done" );
        _dump_polygon( mp_result );
        new_polygon = mpoly_to_poly( mp_result );
        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
            PG_RETURN_NULL();
        }

        pfree( sorted_polys[0] );
        sorted_polys[0] = new_polygon;
    }

    for( i = 0; i < num_poly; i++ )
    {
        pfree( sorted_polys[i] );
    }

    pfree( sorted_polys );

    if( new_polygon == NULL )
    {
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, true );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    PG_RETURN_POLYGON_P( new_polygon );
}

/*
 * Preprocessing frontend that handles buffering, scaling and sorting of polys from function input
 * We CANNOT modify the original polygons, as they may be pointers to something in the disk buffer
 */

POLYGON ** poly_preprocessing(
    ArrayType * polyarray,
    bool sort,
    bool scale,
    int * num_poly
)
{
    POLYGON ** ret               = NULL;
    Datum *    dpoly             = NULL;
    bool *     nulls             = NULL;
    int        i                 = 0;
    int        j                 = 0;
    double     sum_x             = 0;
    double     sum_y             = 0;
    Point   ** centers           = NULL;
    Point    * center            = NULL;
    int        npoints           = 0;
    POLYGON ** buff_polys        = NULL;
    POLYGON ** buff_polys_sorted = NULL;
    POLYGON  * buff_poly         = NULL;
    int        palloc_sz         = 0;
    int        remaining         = 0;
    double     last_dist         = 0;
    double     min_dist          = 0;
    int        last_dist_ind     = 0;
    int        sort_ind          = 0;

    // Values for POLYGON type taken from pg_catalog.pg_type
    deconstruct_array(
        polyarray,
        POLYGONOID,
        -1,
        false,
        'd',
        &dpoly,
        &nulls,
        num_poly
    );

    if( (*num_poly) <= 1 )
    {
        buff_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );
        buff_poly = DatumGetPolygonP( dpoly[0] );
        buff_polys[0] = buff_poly;
        return buff_polys;
    }

    // Allocate structs
    palloc_sz  = sizeof( POLYGON * ) * (*num_poly);
    buff_polys = ( POLYGON ** ) palloc0( palloc_sz );
    palloc_sz  = (*num_poly) * sizeof( Point * );
    centers    = ( Point ** ) palloc0( palloc_sz );

    for( i = 0; i < (*num_poly); i++ )
    {
        POLYGON * current_poly = NULL;
        npoints                = 0;
        sum_x                  = 0;
        sum_y                  = 0;

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

        current_poly = DatumGetPolygonP( dpoly[i] );
        palloc_sz    = offsetof( POLYGON, p )
                     + sizeof( buff_poly->p[0] )
                     * current_poly->npts;
        buff_poly    = ( POLYGON * ) palloc0( palloc_sz );
        center       = ( Point * ) palloc0( palloc_sz );

        for( j = 0; j < current_poly->npts; j++ )
        {
            sum_x += current_poly->p[j].x;
            sum_y += current_poly->p[j].y;
            npoints++;
        }

        center->x = sum_x / npoints;
        center->y = sum_y / npoints;


        for( j = 0; j < current_poly->npts; j++ )
        {
            if( scale )
            {
                buff_poly->p[j].x = current_poly->p[j].x * ZOOM_RATE - ( ZOOM_RATE - 1 ) * center->x;
                buff_poly->p[j].y = current_poly->p[j].y * ZOOM_RATE - ( ZOOM_RATE - 1 ) * center->y;
            }
            else
            {
                buff_poly->p[j].x = current_poly->p[j].x;
                buff_poly->p[j].y = current_poly->p[j].y;
            }
        }

        buff_poly->npts     = current_poly->npts;
        buff_poly->boundbox = current_poly->boundbox;

        buff_polys[i] = buff_poly;
        centers[i]    = center;
    }

    if( sort )
    {
        buff_polys_sorted = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) * (*num_poly) );

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
                    last_dist = distance( centers[0], centers[i] );

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

    for( i = 0; i < (*num_poly); i++ )
    {
        pfree( centers[i] );
    }

    pfree( centers );

    if( buff_polys_sorted != NULL )
    {
        ret = buff_polys_sorted;
    }

    return ret;
}

POLYGON * poly_postprocessing( POLYGON * poly, bool scale )
{
    int     i      = 0;
    Point * center = NULL;
    double  sum_x  = 0;
    double  sum_y  = 0;

    if( poly == NULL )
    {
        return NULL;
    }

    if( scale )
    {
        // De-scale the poly by ZOOM_RATE
        center = ( Point * ) palloc0( sizeof( Point ) );

        for( i = 0; i < poly->npts; i++ )
        {
            sum_x += poly->p[i].x;
            sum_y += poly->p[i].y;
        }

        center->x = sum_x / poly->npts;
        center->y = sum_y / poly->npts;

        for( i = 0; i < poly->npts; i++ )
        {
            poly->p[i].x = ( poly->p[i].x + ( ZOOM_RATE - 1 ) * center->x ) / ZOOM_RATE;
            poly->p[i].y = ( poly->p[i].y + ( ZOOM_RATE - 1 ) * center->y ) / ZOOM_RATE;
        }

        pfree( center );
    }

    // TODO: Eliminate duplicate points and points that are coincident / colinear
    return poly;
}

static void dump_polygon( POLYGON * p )
{
    int i;

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
