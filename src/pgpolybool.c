/*------------------------------------------------------------------------------
 * pgpolybool.c
 *      PostgreSQL function declarations
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      pgpolybool.c
 *
 *------------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/array.h"
#include "utils/geo_decls.h"
#include "catalog/pg_type.h"
#include "utils/lsyscache.h"
#include "fmgr.h"

#include "martinez.h"
#include "polyprocessing.h"
#include "lseg_funcs.h"
#include "box_funcs.h"
#include "poly_funcs.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

// Intersection
PG_FUNCTION_INFO_V1( fn_intersect_polygons_array );
PG_FUNCTION_INFO_V1( fn_intersect_polygons );

// Difference / Subtraction
PG_FUNCTION_INFO_V1( fn_subtract_polygons_array );
PG_FUNCTION_INFO_V1( fn_subtract_polygons );

// Union
PG_FUNCTION_INFO_V1( fn_union_polygons_array );
PG_FUNCTION_INFO_V1( fn_union_polygons );

// Exclusive Or
PG_FUNCTION_INFO_V1( fn_xor_polygons_array );
PG_FUNCTION_INFO_V1( fn_xor_polygons );

// Other random ops
// Polygon Functions
PG_FUNCTION_INFO_V1( fn_rotate_polygon );
PG_FUNCTION_INFO_V1( fn_get_polygon_points );
PG_FUNCTION_INFO_V1( fn_get_polygon_line_segs );
PG_FUNCTION_INFO_V1( fn_get_polygon_lseg_distance );
PG_FUNCTION_INFO_V1( fn_get_polygon_area );
PG_FUNCTION_INFO_V1( fn_lseg_to_polygon );
PG_FUNCTION_INFO_V1( fn_box_to_polygon );
PG_FUNCTION_INFO_V1( fn_points_to_polygon );

// LSEG functions
PG_FUNCTION_INFO_V1( fn_get_parallel_segment );
PG_FUNCTION_INFO_V1( fn_get_parallel_segments );
PG_FUNCTION_INFO_V1( fn_get_orthogonal_segment );
PG_FUNCTION_INFO_V1( fn_get_orthogonal_segments );
PG_FUNCTION_INFO_V1( fn_scale_lseg );
PG_FUNCTION_INFO_V1( fn_get_lseg_angle );
PG_FUNCTION_INFO_V1( fn_lseg_points_right_of );
PG_FUNCTION_INFO_V1( fn_lseg_points_left_of );
PG_FUNCTION_INFO_V1( fn_cross_product );
PG_FUNCTION_INFO_V1( fn_lseg_to_vector );

Datum fn_subtract_polygons_array( PG_FUNCTION_ARGS )
{
    POLYGON **       sorted_polys = NULL;
    POLYGON **       result_polys = NULL;
    POLYGON *        new_polygon  = NULL;
    struct polygon * mp_subj      = NULL;
    struct polygon * mp_clip      = NULL;
    struct polygon * mp_result    = NULL;
    unsigned int     num_poly     = 0;
    unsigned int     i            = 0;
    Point **         centers      = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing_array(
        PG_GETARG_ARRAYTYPE_P(0),
        false,
        false,
        &centers,
        &num_poly
    );

    // Free centers, we only want sorting
    if( centers != NULL )
    {
        for( i = 0; i < num_poly; i++ )
        {
            if( centers[i] != NULL )
            {
                pfree( centers[i] );
                centers[i] = NULL;
            }
        }

        pfree( centers );
        centers = NULL;
    }

    if( sorted_polys == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure" );
    }

    if( num_poly == 1 )
    {
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    // Allocate space for our result
    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

    new_polygon = sorted_polys[0];

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( new_polygon );
        dump_polygon( sorted_polys[i] );
#endif
        // We're going to re-use new_polygon for our running result
        mp_subj = poly_to_mpoly( new_polygon );
        new_polygon = NULL;
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        sorted_polys[i] = NULL;
        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_DIFFERENCE
        );

        if( mp_result == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        result_polys = mpoly_to_poly( mp_result, true );

        if( result_polys == NULL )
        {
            elog( ERROR, "Polygon type conversion failed" );
        }

        new_polygon = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
    }

    pfree( sorted_polys );
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        PG_RETURN_NULL();
    }

    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_subtract_polygons( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys  = NULL;
    POLYGON *        new_polygon   = NULL;
    POLYGON *        buff_polys[2] = {NULL};
    struct polygon * mp_subj       = NULL;
    struct polygon * mp_clip       = NULL;
    struct polygon * mp_result     = NULL;
    Point *          centers[2]    = {NULL};

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    buff_polys[0] = poly_preprocessing(
        PG_GETARG_POLYGON_P(0),
        false,
        &centers[0]
    );

    buff_polys[1] = poly_preprocessing(
        PG_GETARG_POLYGON_P(1),
        false,
        &centers[1]
    );

    if( centers[0] != NULL )
    {
        pfree( centers[0] );
    }

    if( centers[1] != NULL )
    {
        pfree( centers[1] );
    }

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

    if( buff_polys[0] == NULL || buff_polys[1] == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure" );
    }

#ifdef DEBUG
    dump_polygon( buff_polys[0] );
    dump_polygon( buff_polys[1] );
#endif
    // Type conversion prior to computation
    mp_subj = poly_to_mpoly( buff_polys[0] );
    mp_clip = poly_to_mpoly( buff_polys[1] );

    buff_polys[0] = NULL;
    buff_polys[1] = NULL;

    mp_result = compute(
        mp_subj,
        mp_clip,
        OP_DIFFERENCE
    );

    if( mp_result == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );
        PG_RETURN_NULL();
    }

    result_polys = mpoly_to_poly( mp_result, true );

    if( result_polys == NULL )
    {
        elog( ERROR, "Polygon type conversion failed" );
    }

    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );

    if( new_polygon == NULL )
    {
        elog( ERROR, "Polygon post-processing error" );
    }

    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_intersect_polygons_array( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys = NULL;
    POLYGON **       sorted_polys = NULL;
    POLYGON *        new_polygon  = NULL;
    struct polygon * mp_subj      = NULL;
    struct polygon * mp_clip      = NULL;
    struct polygon * mp_result    = NULL;
    unsigned int     num_poly     = 0;
    unsigned int     i            = 0;
    Point **         centers      = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing_array(
        PG_GETARG_ARRAYTYPE_P(0),
        true,
        false,
        &centers,
        &num_poly
    );

    if( centers != NULL )
    {
        for( i = 0; i < num_poly; i++ )
        {
            if( centers[i] != NULL )
            {
                pfree( centers[i] );
                centers[i] = NULL;
            }
        }

        pfree( centers );
        centers = NULL;
    }

    if( sorted_polys == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure"
        );
    }

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

    if( num_poly == 1 )
    {
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    new_polygon = sorted_polys[0];

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( new_polygon );
        dump_polygon( sorted_polys[i] );
#endif
        mp_subj = poly_to_mpoly( sorted_polys[0] );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        sorted_polys[0] = NULL;
        sorted_polys[i] = NULL;
        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_INTERSECTION
        );

        if( mp_result == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        result_polys = mpoly_to_poly( mp_result, true );

        if( result_polys == NULL )
        {
            elog( ERROR, "Polygon type conversion failed" );
        }

        new_polygon = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
    }

    pfree( sorted_polys );
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        PG_RETURN_NULL();
    }

    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_intersect_polygons( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys  = NULL;
    POLYGON *        new_polygon   = NULL;
    POLYGON *        buff_polys[2] = {NULL};
    struct polygon * mp_subj       = NULL;
    struct polygon * mp_clip       = NULL;
    struct polygon * mp_result     = NULL;
    Point *          centers[2]    = {NULL};

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    buff_polys[0] = poly_preprocessing(
        PG_GETARG_POLYGON_P(0),
        false,
        &centers[0]
    );

    buff_polys[1] = poly_preprocessing(
        PG_GETARG_POLYGON_P(1),
        false,
        &centers[1]
    );

    if( centers[0] != NULL )
    {
        pfree( centers[0] );
    }

    if( centers[1] != NULL )
    {
        pfree( centers[1] );
    }

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

    if( buff_polys[0] == NULL || buff_polys[1] == NULL )
    {
        elog( ERROR, "Polygon post-processing failure" );
    }

#ifdef DEBUG
    dump_polygon( buff_polys[0] );
    dump_polygon( buff_polys[1] );
#endif
    mp_subj = poly_to_mpoly( buff_polys[0] );
    mp_clip = poly_to_mpoly( buff_polys[1] );
    buff_polys[0] = NULL;
    buff_polys[1] = NULL;

    mp_result = compute(
        mp_subj,
        mp_clip,
        OP_INTERSECTION
    );

    elog( DEBUG1, "Result is %p", mp_result );

    if( mp_result == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );
        PG_RETURN_NULL();
    }

    result_polys = mpoly_to_poly( mp_result, true );

    if( result_polys == NULL )
    {
        elog( ERROR, "Polygon type conversion failed" );
    }

    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    if( new_polygon == NULL )
    {
        elog( ERROR, "Polygon post-processing error" );
    }

    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_union_polygons_array( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys = NULL;
    POLYGON **       sorted_polys = NULL;
    POLYGON *        new_polygon  = NULL;
    struct polygon * mp_subj      = NULL;
    struct polygon * mp_clip      = NULL;
    struct polygon * mp_result    = NULL;
    unsigned int     num_poly     = 0;
    unsigned int     i            = 0;
    Point **         centers      = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing_array(
        PG_GETARG_ARRAYTYPE_P(0),
        true,
        true,
        &centers,
        &num_poly
    );

    if( sorted_polys == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure" );
    }

    if( num_poly == 1 )
    {
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

    new_polygon = sorted_polys[0];

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( new_polygon );
        dump_polygon( sorted_polys[i] );
#endif // DEBUG
        mp_subj = poly_to_mpoly( new_polygon );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        new_polygon     = NULL;
        sorted_polys[i] = NULL;
        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_UNION
        );

#ifdef DEBUG
        elog( DEBUG1, "Compute with OP_UNION done" );
        _dump_polygon( mp_result );
#endif // DEBUG
        if( mp_result == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        result_polys = mpoly_to_poly( mp_result, true );

        if( result_polys == NULL )
        {
            elog( ERROR, "Polygon type conversion failed" );
        }

        new_polygon  = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
    }

    pfree( sorted_polys );
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        PG_RETURN_NULL();
    }

    // De-scale polygon prior to output
    new_polygon = poly_postprocessing( new_polygon, centers, num_poly, true );

    if( centers != NULL )
    {
        for( i = 0; i < num_poly; i++ )
        {
            if( centers[i] != NULL )
            {
                pfree( centers[i] );
                centers[i] = NULL;
            }
        }

        pfree( centers );
        centers = NULL;
    }

    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif

    if( new_polygon == NULL )
    {
        elog( ERROR, "Polygon post-processing error" );
    }

    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_union_polygons( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys  = NULL;
    POLYGON *        new_polygon   = NULL;
    POLYGON *        buff_polys[2] = {NULL};
    struct polygon * mp_subj       = NULL;
    struct polygon * mp_clip       = NULL;
    struct polygon * mp_result     = NULL;
    Point *          centers[2]    = {NULL};

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    buff_polys[0] = poly_preprocessing(
        PG_GETARG_POLYGON_P(0),
        true,
        &centers[0]
    );

    buff_polys[1] = poly_preprocessing(
        PG_GETARG_POLYGON_P(1),
        true,
        &centers[1]
    );

    if( buff_polys[0] == NULL || buff_polys[1] == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure" );
    }

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

#ifdef DEBUG
    dump_polygon( buff_polys[0] );
    dump_polygon( buff_polys[1] );
#endif
    mp_subj = poly_to_mpoly( buff_polys[0] );
    mp_clip = poly_to_mpoly( buff_polys[1] );

    mp_result = compute(
        mp_subj,
        mp_clip,
        OP_UNION
    );

    if( mp_result == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );
        PG_RETURN_NULL();
    }

    result_polys = mpoly_to_poly( mp_result, true );

    if( result_polys == NULL )
    {
        elog( ERROR, "Polygon type conversion failed" );
    }

    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );

        if( centers[0] != NULL )
        {
            pfree( centers[0] );
        }

        if( centers[1] != NULL )
        {
            pfree( centers[1] );
        }

        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, centers, 2, true );

    if( centers[0] != NULL )
    {
        pfree( centers[0] );
        centers[0] = NULL;
    }

    if( centers[1] != NULL )
    {
        pfree( centers[1] );
        centers[1] = NULL;
    }

    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    if( new_polygon == NULL )
    {
        elog( ERROR, "Polygon post-processing error" );
    }

    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_xor_polygons_array( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys = NULL;
    POLYGON **       sorted_polys = NULL;
    POLYGON *        new_polygon  = NULL;
    struct polygon * mp_subj      = NULL;
    struct polygon * mp_clip      = NULL;
    struct polygon * mp_result    = NULL;
    unsigned int     num_poly     = 0;
    unsigned int     i            = 0;
    Point **         centers      = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    sorted_polys = poly_preprocessing_array(
        PG_GETARG_ARRAYTYPE_P(0),
        true,
        false,
        &centers,
        &num_poly
    );

    if( sorted_polys == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure" );
        PG_RETURN_NULL();
    }

    if( num_poly == 1 )
    {
        new_polygon = sorted_polys[0];
        pfree( sorted_polys );

        if( new_polygon == NULL )
        {
            PG_RETURN_NULL();
        }

        PG_RETURN_POLYGON_P( new_polygon );
    }

    new_polygon = sorted_polys[0];

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

    for( i = 1; i < num_poly; i++ )
    {
#ifdef DEBUG
        dump_polygon( new_polygon );
        dump_polygon( sorted_polys[i] );
#endif
        mp_subj = poly_to_mpoly( new_polygon );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        new_polygon     = NULL;
        sorted_polys[i] = NULL;
        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_XOR
        );

#ifdef DEBUG
        elog( DEBUG1, "Compute with OP_XOR done" );
        _dump_polygon( mp_result );
#endif // DEBUG

        if( mp_result == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        result_polys = mpoly_to_poly( mp_result, true );

        if( result_polys == NULL )
        {
            elog( ERROR, "Polygon type conversion failed" );
        }

        new_polygon  = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( DEBUG, "Polygons have no intersection" );
            PG_RETURN_NULL();
        }

        new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
    }

    pfree( sorted_polys );
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        if( centers != NULL )
        {
            for( i = 0; i < num_poly; i++ )
            {
                if( centers[i] != NULL )
                {
                    pfree( centers[i] );
                    centers[i] = NULL;
                }
            }

            pfree( centers );
            centers = NULL;
        }
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, centers, num_poly, false );
    set_polygon_boundbox( new_polygon );

    if( centers != NULL )
    {
        for( i = 0; i < num_poly; i++ )
        {
            if( centers[i] != NULL )
            {
                pfree( centers[i] );
                centers[i] = NULL;
            }
        }

        pfree( centers );
        centers = NULL;
    }
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif

    if( new_polygon == NULL )
    {
        elog( ERROR, "Polygon post-processing error" );
    }

    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

Datum fn_xor_polygons( PG_FUNCTION_ARGS )
{
    POLYGON **       result_polys  = NULL;
    POLYGON *        new_polygon   = NULL;
    POLYGON *        buff_polys[2] = {NULL};
    struct polygon * mp_subj       = NULL;
    struct polygon * mp_clip       = NULL;
    struct polygon * mp_result     = NULL;
    Point *          centers[2]    = {NULL};

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    buff_polys[0] = poly_preprocessing(
        PG_GETARG_POLYGON_P(0),
        false,
        &centers[0]
    );

    buff_polys[1] = poly_preprocessing(
        PG_GETARG_POLYGON_P(1),
        false,
        &centers[1]
    );

    if( buff_polys[0] == NULL || buff_polys[1] == NULL )
    {
        elog( ERROR, "Polygon pre-processing failure" );
    }

    result_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

    if( result_polys == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create result polygon array" )
            )
        );
    }

#ifdef DEBUG
    dump_polygon( buff_polys[0] );
    dump_polygon( buff_polys[1] );
#endif
    mp_subj = poly_to_mpoly( buff_polys[0] );
    mp_clip = poly_to_mpoly( buff_polys[1] );
    buff_polys[0] = NULL;
    buff_polys[1] = NULL;
    mp_result = compute(
        mp_subj,
        mp_clip,
        OP_XOR
    );

    if( mp_result == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );
        PG_RETURN_NULL();
    }

    result_polys = mpoly_to_poly( mp_result, false );

    if( result_polys == NULL )
    {
        elog( ERROR, "Polygon type conversion failed" );
    }

    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( DEBUG, "Polygons have no intersection" );

        if( centers[0] != NULL )
        {
            pfree( centers[0] );
            centers[0] = NULL;
        }

        if( centers[1] != NULL )
        {
            pfree( centers[1] );
            centers[1] = NULL;
        }

        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, centers, 2, true );

    if( centers[0] != NULL )
    {
        pfree( centers[0] );
        centers[0] = NULL;
    }

    if( centers[1] != NULL )
    {
        pfree( centers[1] );
        centers[1] = NULL;
    }

    set_polygon_boundbox( new_polygon );
#ifdef DEBUG
    dump_polygon( new_polygon );
#endif
    if( new_polygon == NULL )
    {
        elog( ERROR, "Polygon post-processing error" );
    }

    SET_VARSIZE(
        new_polygon,
        offsetof( POLYGON, p )
      + ( new_polygon->npts * sizeof( Point ) )
    );

    PG_RETURN_POLYGON_P( new_polygon );
}

/*
 * Polygon Helper Functions
 */

Datum fn_rotate_polygon( PG_FUNCTION_ARGS )
{
    POLYGON * poly    = NULL;
    float8    radians = 0.0;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    poly    = PG_GETARG_POLYGON_P_COPY(0);
    radians = PG_GETARG_FLOAT8(1);

    rotate_polygon( poly, radians );

    PG_RETURN_POLYGON_P( poly );
}

Datum fn_get_polygon_points( PG_FUNCTION_ARGS )
{
    POLYGON *    poly      = NULL;
    unsigned int i         = 0;
    ArrayType *  result    = NULL;
    Datum *      elements  = NULL;
    Point **     p_result  = NULL;
    int16        typlen;
    char         typalign;
    bool         typbyval;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    poly = PG_GETARG_POLYGON_P(0);

    if( poly == NULL )
    {
        PG_RETURN_NULL();
    }

    p_result = get_polygon_points( poly );

    if( p_result == NULL )
    {
        PG_RETURN_NULL();
    }

    elements = ( Datum * ) palloc0( sizeof( Datum ) * poly->npts );

    if( elements == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not allocate polygon point return array" )
            )
        );
    }

    for( i = 0; i < poly->npts; i++ )
    {
        elements[i] = PointPGetDatum( p_result[i] );
    }

    get_typlenbyvalalign( POINTOID, &typlen, &typbyval, &typalign );

    result = construct_array(
        elements,
        poly->npts,
        POINTOID,
        typlen,
        typbyval,
        typalign
    );

    PG_RETURN_ARRAYTYPE_P( result );
}

Datum fn_get_polygon_line_segs( PG_FUNCTION_ARGS )
{
    POLYGON *    poly     = NULL;
    Datum *      elements = NULL;
    ArrayType *  result   = NULL;
    unsigned int i        = 0;
    unsigned int next_i   = 0;
    LSEG *       segment  = NULL;

    int16 typlen;
    char  typalign;
    bool  typbyval;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    poly = PG_GETARG_POLYGON_P(0);

    if( poly == NULL )
    {
        PG_RETURN_NULL();
    }

    if( poly->npts == 1 )
    {
        PG_RETURN_NULL();
    }

    elements = ( Datum * ) palloc0( sizeof( Datum ) * poly->npts );

    if( elements == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg(
                    "Could not allocate polygon line segment return array"
                )
            )
        );
    }

    for( i = 0; i < poly->npts; i++ )
    {
        if( i == poly->npts - 1 )
        {
            next_i = 0;
        }
        else
        {
            next_i = i + 1;
        }

        segment = ( LSEG * ) palloc0( sizeof( LSEG ) );

        segment->p[0] = poly->p[i];
        segment->p[1] = poly->p[next_i];
        elements[i] = LsegPGetDatum( segment );
    }

    get_typlenbyvalalign( LSEGOID, &typlen, &typbyval, &typalign );
    result = construct_array(
        elements,
        poly->npts,
        LSEGOID,
        typlen,
        typbyval,
        typalign
    );

    PG_RETURN_ARRAYTYPE_P( result );
}

Datum fn_get_polygon_lseg_distance( PG_FUNCTION_ARGS )
{
    POLYGON *    poly     = NULL;
    LSEG *       in_lseg  = NULL;
    LSEG *       segment  = NULL;
    unsigned int i        = 0;
    unsigned int next_i   = 0;
    double       min_dist = 0.0;
    double       dist     = 0.0;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    poly    = PG_GETARG_POLYGON_P(0);
    in_lseg = PG_GETARG_LSEG_P(1);

    if( poly == NULL || in_lseg == NULL )
    {
        PG_RETURN_NULL();
    }

    if( poly->npts == 1 )
    {
        PG_RETURN_NULL();
    }

    min_dist = DBL_MAX;
    segment  = ( LSEG * ) palloc0( sizeof( LSEG ) );

    if( segment == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not divide polygon into line segment" )
            )
        );
    }

    for( i = 0; i < poly->npts; i++ )
    {
        if( i == poly->npts - 1 )
        {
            next_i = 0;
        }
        else
        {
            next_i = i + 1;
        }


        segment->p[0] = poly->p[i];
        segment->p[1] = poly->p[next_i];

        dist = line_segment_distance( segment, in_lseg );

        if( dist < min_dist )
        {
            min_dist = dist;
        }
    }

    pfree( segment );

    PG_RETURN_FLOAT8( min_dist );
}

Datum fn_get_parallel_segment( PG_FUNCTION_ARGS )
{
    LSEG *  seg        = NULL;
    Point * away_point = NULL;
    LSEG ** result     = NULL;
    LSEG *  result_p   = NULL;
    double  distance   = 1.0;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    seg = PG_GETARG_LSEG_P(0);

    if( seg == NULL )
    {
        PG_RETURN_NULL();
    }

    if( !PG_ARGISNULL(1) )
    {
        away_point = PG_GETARG_POINT_P(1);

        if( away_point == NULL )
        {
            PG_RETURN_NULL();
        }
    }

    if( !PG_ARGISNULL(2) )
    {
        distance = PG_GETARG_FLOAT8(2);
    }

    result = line_segment_parallel_line_segment( seg, away_point, distance );

    if( result == NULL )
    {
        PG_RETURN_NULL();
    }
    else
    {
        result_p = result[0];

        if( PG_ARGISNULL(1) )
        {
            pfree( result[1] );
        }

        pfree( result );

        PG_RETURN_LSEG_P( result_p );
    }
}

Datum fn_get_parallel_segments( PG_FUNCTION_ARGS )
{
    LSEG *      seg      = NULL;
    LSEG **     l_result = NULL;
    Datum *     elements = NULL;
    ArrayType * result   = NULL;
    double      distance = 1.0;

    int16 typlen;
    char  typalign;
    bool  typbyval;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    seg = PG_GETARG_LSEG_P(0);

    if( seg == NULL )
    {
        PG_RETURN_NULL();
    }

    if( !PG_ARGISNULL(1) )
    {
        distance = PG_GETARG_FLOAT8(1);
    }

    l_result = line_segment_parallel_line_segment( seg, NULL, distance );

    if( l_result == NULL )
    {
        PG_RETURN_NULL();
    }

    elements = ( Datum * ) palloc0( sizeof( Datum ) * 2 );

    if( elements == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg(
                    "Could not allocate polygon line segment return array"
                )
            )
        );
    }

    elements[0] = LsegPGetDatum( l_result[0] );
    elements[1] = LsegPGetDatum( l_result[1] );

    pfree( l_result[0] );
    pfree( l_result[1] );
    pfree( l_result );

    get_typlenbyvalalign( LSEGOID, &typlen, &typbyval, &typalign );

    result = construct_array(
        elements,
        2,
        LSEGOID,
        typlen,
        typbyval,
        typalign
    );

    PG_RETURN_ARRAYTYPE_P( result );
}

Datum fn_get_orthogonal_segment( PG_FUNCTION_ARGS )
{
    LSEG *  seg        = NULL;
    Point * away_point = NULL;
    LSEG ** result     = NULL;
    LSEG *  result_p   = NULL;
    double  length     = 1.0;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    seg = PG_GETARG_LSEG_P(0);

    if( seg == NULL )
    {
        PG_RETURN_NULL();
    }

    if( !PG_ARGISNULL(1) )
    {
        away_point = PG_GETARG_POINT_P(1);

        if( away_point == NULL )
        {
            PG_RETURN_NULL();
        }
    }

    if( !PG_ARGISNULL(2) )
    {
        length = PG_GETARG_FLOAT8(2);
    }

    result = line_segment_orthogonal_line_segment( seg, away_point, length );

    if( result == NULL )
    {
        PG_RETURN_NULL();
    }
    else
    {
        result_p = result[0];

        if( PG_ARGISNULL(1) )
        {
            pfree( result[1] );
        }

        pfree( result );

        PG_RETURN_LSEG_P( result_p );
    }
}

Datum fn_get_orthogonal_segments( PG_FUNCTION_ARGS )
{
    LSEG *      seg      = NULL;
    LSEG **     l_result = NULL;
    Datum *     elements = NULL;
    ArrayType * result   = NULL;
    double      length   = 1.0;

    int16 typlen;
    char  typalign;
    bool  typbyval;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    seg = PG_GETARG_LSEG_P(0);

    if( seg == NULL )
    {
        PG_RETURN_NULL();
    }

    if( !PG_ARGISNULL(1) )
    {
        length = PG_GETARG_FLOAT8(1);
    }

    l_result = line_segment_orthogonal_line_segment( seg, NULL, length );

    if( l_result == NULL )
    {
        PG_RETURN_NULL();
    }

    elements = ( Datum * ) palloc0( sizeof( Datum ) * 2 );

    if( elements == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg(
                    "Could not allocate polygon line segment return array"
                )
            )
        );
    }

    elements[0] = LsegPGetDatum( l_result[0] );
    elements[1] = LsegPGetDatum( l_result[1] );

    pfree( l_result[0] );
    pfree( l_result[1] );
    pfree( l_result );

    get_typlenbyvalalign( LSEGOID, &typlen, &typbyval, &typalign );

    result = construct_array(
        elements,
        2,
        LSEGOID,
        typlen,
        typbyval,
        typalign
    );

    PG_RETURN_ARRAYTYPE_P( result );
}

Datum fn_lseg_to_polygon( PG_FUNCTION_ARGS )
{
    POLYGON * poly    = NULL;
    LSEG *    segment = NULL;
    double    width   = 0.0;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    segment = PG_GETARG_LSEG_P(0);
    width   = PG_GETARG_FLOAT8(1);

    if( segment == NULL )
    {
        PG_RETURN_NULL();
    }

    poly = line_segment_to_polygon( segment, width );

    if( poly == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_POLYGON_P( poly );
}

Datum fn_get_polygon_area( PG_FUNCTION_ARGS )
{
    POLYGON * poly = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    poly = PG_GETARG_POLYGON_P(0);

    if( poly == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_FLOAT8( get_polygon_area( poly ) );
}

Datum fn_scale_lseg( PG_FUNCTION_ARGS )
{
    LSEG *  segment = NULL;
    float8  scale   = 0.0;
    Point * ref     = NULL;
    LSEG *  result  = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    segment = PG_GETARG_LSEG_P(0);

    if( segment == NULL )
    {
        PG_RETURN_NULL();
    }

    if( !PG_ARGISNULL(1) )
    {
        scale = PG_GETARG_FLOAT8(1);
    }

    if( !PG_ARGISNULL(2) )
    {
        ref = PG_GETARG_POINT_P(2);
    }

    result = scale_lseg( segment, scale, ref );

    if( result == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_LSEG_P( result );
}

Datum fn_get_lseg_angle( PG_FUNCTION_ARGS )
{
    LSEG * segment_a = NULL;
    LSEG * segment_b = NULL;
    float8 result    = 0.0;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    segment_a = PG_GETARG_LSEG_P(0);
    segment_b = PG_GETARG_LSEG_P(1);

    if( segment_a == NULL || segment_b == NULL )
    {
        PG_RETURN_NULL();
    }

    result = get_angle_of_intersection(
        segment_a,
        segment_b
    );

    PG_RETURN_FLOAT8( result );
}

Datum fn_lseg_points_right_of( PG_FUNCTION_ARGS )
{
    LSEG * seg = NULL;
    LSEG * ref = NULL;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    seg = PG_GETARG_LSEG_P(0);
    ref = PG_GETARG_LSEG_P(1);

    if( seg == NULL || ref == NULL )
    {
        PG_RETURN_NULL();
    }

    return lseg_points_right_of( seg, ref );
}

Datum fn_lseg_points_left_of( PG_FUNCTION_ARGS )
{
    LSEG * seg = NULL;
    LSEG * ref = NULL;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    seg = PG_GETARG_LSEG_P(0);
    ref = PG_GETARG_LSEG_P(1);

    if( seg == NULL || ref == NULL )
    {
        PG_RETURN_NULL();
    }

    return lseg_points_left_of( seg, ref );
}

Datum fn_cross_product( PG_FUNCTION_ARGS )
{
    Point * a = NULL;
    Point * b = NULL;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    a = PG_GETARG_POINT_P(0);
    b = PG_GETARG_POINT_P(1);

    if( a == NULL || b == NULL )
    {
        PG_RETURN_NULL();
    }

    return cross_product( a, b );
}

Datum fn_lseg_to_vector( PG_FUNCTION_ARGS )
{
    Point * result = NULL;
    LSEG *  input  = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    input = PG_GETARG_LSEG_P(0);

    if( input == NULL )
    {
        PG_RETURN_NULL();
    }

    result = lseg_to_vector( input );

    if( result == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_POINT_P( result );
}

Datum fn_box_to_polygon( PG_FUNCTION_ARGS )
{
    POLYGON * result = NULL;
    BOX *     input  = NULL;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    input = PG_GETARG_BOX_P(0);

    if( input == NULL )
    {
        PG_RETURN_NULL();
    }

    result = box_to_polygon( input );

    if( result == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_POLYGON_P( result );
}

Datum fn_points_to_polygon( PG_FUNCTION_ARGS )
{
    POLYGON *    result       = NULL;
    ArrayType *  input_points = NULL;
    Datum *      elements     = NULL;
    Point **     input        = NULL;
    bool *       nulls        = NULL;
    int          num_points   = 0;
    unsigned int i            = 0;

    if( PG_ARGISNULL(0) )
    {
        PG_RETURN_NULL();
    }

    input_points = PG_GETARG_ARRAYTYPE_P(0);

    if( input_points == NULL )
    {
        PG_RETURN_NULL();
    }

    deconstruct_array(
        input_points,
        POINTOID,
        16,
        false,
        'd',
        &elements,
        &nulls,
        &num_points
    );

    if( num_points > 1 )
    {
        input = ( Point ** ) palloc0(
            sizeof( Point * ) * num_points
        );

        if( input == NULL )
        {
            PG_RETURN_NULL();
        }

        for( i = 0; i < num_points; i++ )
        {
            if( nulls[i] )
            {
                pfree( input );
                PG_RETURN_NULL();
            }

            input[i] = DatumGetPointP( elements[i] );
        }
    }
    else
    {
        PG_RETURN_NULL();
    }

    result = polygon_from_points(
        input,
        ( unsigned int ) num_points
    );

    if( result == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_POLYGON_P( result );
}
