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
// LSEG functions
PG_FUNCTION_INFO_V1( fn_lseg_intersect );
PG_FUNCTION_INFO_V1( fn_lseg_intersect_point );
PG_FUNCTION_INFO_V1( fn_lseg_distance );
PG_FUNCTION_INFO_V1( fn_get_orthogonal_segment );
PG_FUNCTION_INFO_V1( fn_get_orthogonal_segments );

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

    for( i = 0; i < num_poly; i++ )
    {
        pfree( centers[i] );
    }

    pfree( centers );

    if( sorted_polys == NULL )
    {
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
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
        mp_subj = poly_to_mpoly( new_polygon );
        mp_clip = poly_to_mpoly( sorted_polys[i] );
        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_DIFFERENCE
        );

        result_polys = mpoly_to_poly( mp_result, true );
        new_polygon  = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
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

    pfree( centers[0] );
    pfree( centers[1] );

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
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
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
        OP_DIFFERENCE
    );

    result_polys = mpoly_to_poly( mp_result, true );
    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( WARNING, "polygons have no intersections" );
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
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

    for( i = 0; i < num_poly; i++ )
    {
        pfree( centers[i] );
    }

    pfree( centers );

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

    if( sorted_polys == NULL )
    {
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
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
        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_INTERSECTION
        );

        result_polys = mpoly_to_poly( mp_result, true );
        new_polygon  = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
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

    pfree( centers[0] );
    pfree( centers[1] );

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
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
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
        OP_INTERSECTION
    );

    result_polys = mpoly_to_poly( mp_result, true );
    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( WARNING, "polygons have no intersections" );
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, NULL, 0, false );
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
        elog( WARNING, "poly preprocessing failure" );
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
#endif // DEBUG
        mp_subj = poly_to_mpoly( new_polygon );
        mp_clip = poly_to_mpoly( sorted_polys[i] );

        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_UNION
        );

#ifdef DEBUG
        elog( DEBUG1, "Compute with OP_UNION done" );
        _dump_polygon( mp_result );
#endif // DEBUG
        result_polys = mpoly_to_poly( mp_result, true );
        new_polygon  = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
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

    for( i = 0; i < num_poly; i++ )
    {
        pfree( centers[i] );
    }

    pfree( centers );
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
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
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

    result_polys = mpoly_to_poly( mp_result, true );
    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( WARNING, "polygons have no intersections" );
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, centers, 2, true );

    pfree( centers[0] );
    pfree( centers[1] );
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
        elog( WARNING, "poly preprocessing failure" );
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

        mp_result = compute(
            mp_subj,
            mp_clip,
            OP_XOR
        );

#ifdef DEBUG
        elog( DEBUG1, "Compute with OP_XOR done" );
        _dump_polygon( mp_result );
#endif // DEBUG

        result_polys = mpoly_to_poly( mp_result, true );
        new_polygon  = result_polys[0];

        if( new_polygon == NULL )
        {
            elog( WARNING, "polygons have no intersections" );
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

    new_polygon = poly_postprocessing( new_polygon, centers, num_poly, false );

    for( i = 0; i < num_poly; i++ )
    {
        pfree( centers[i] );
    }

    pfree( centers );
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
        elog( WARNING, "poly preprocessing failure" );
        PG_RETURN_NULL();
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
        OP_XOR
    );

    result_polys = mpoly_to_poly( mp_result, false );
    new_polygon  = result_polys[0];
    pfree( result_polys );

    if( new_polygon == NULL )
    {
        elog( WARNING, "polygons have no intersections" );
        PG_RETURN_NULL();
    }

    new_polygon = poly_postprocessing( new_polygon, centers, 2, true );
    pfree( centers[0] );
    pfree( centers[1] );
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

/*
 * Polygon Helper Functions
 */

Datum fn_rotate_polygon( PG_FUNCTION_ARGS )
{
    POLYGON *    poly     = NULL;
    float8       radians  = 0.0;
    int32        i        = 0;
    Point        center   = {0.0};
    float8       min_x    = DBL_MAX;
    float8       max_x    = -DBL_MAX;
    float8       min_y    = DBL_MAX;
    float8       max_y    = -DBL_MAX;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    poly    = PG_GETARG_POLYGON_P_COPY(0);
    radians = PG_GETARG_FLOAT8(1);

    if(
            radians > ( 2 * PI + DBL_EPSILON )
         || radians < - ( 2 * PI - DBL_EPSILON )
      )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE ),
                errmsg( "radians argument must be between -2pi and 2pi" )
            )
        );
    }

    if( poly == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_NULL_VALUE_NOT_ALLOWED ),
                errmsg( "Polygon may not be NULL" )
            )
        );
    }

    if( fabs( radians ) <= DBL_EPSILON )
    {
        elog( DEBUG1, "Radians <= 0" );
        PG_RETURN_POLYGON_P( poly );
    }

    // Get center from boundingbox
    center.x = ( poly->boundbox.high.x + poly->boundbox.low.x ) / 2;
    center.y = ( poly->boundbox.high.y + poly->boundbox.low.y ) / 2;

    for( i = 0; i < poly->npts; i++ )
    {
        poly->p[i].x = ( poly->p[i].x * cos( radians ) )
                     - ( poly->p[i].y * sin( radians ) )
                     + (
                          center.x
                        - (
                              center.x * cos( radians )
                            - center.y * sin( radians )
                          )
                       );
        poly->p[i].y = ( poly->p[i].x * sin( radians ) )
                     + ( poly->p[i].y * cos( radians ) )
                     + (
                          center.y
                        - (
                              center.x * sin( radians )
                            + center.y * cos( radians )
                          )
                       );

        // Generate new bounding box inline
        max_x = ( poly->p[i].x > max_x ) ? poly->p[i].x : max_x;
        min_x = ( poly->p[i].x < min_x ) ? poly->p[i].x : min_x;
        max_y = ( poly->p[i].y > max_y ) ? poly->p[i].y : max_y;
        min_y = ( poly->p[i].y < min_y ) ? poly->p[i].y : min_y;
    }

    poly->boundbox.high.x = max_x;
    poly->boundbox.high.y = max_y;
    poly->boundbox.low.x  = min_x;
    poly->boundbox.low.y  = min_y;

    PG_RETURN_POLYGON_P( poly );
}

Datum fn_get_polygon_points( PG_FUNCTION_ARGS )
{
    POLYGON *    poly      = NULL;
    unsigned int i         = 0;
    ArrayType *  result    = NULL;
    Datum *      elements  = NULL;

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
        elements[i] = PointPGetDatum( &(poly->p[i]) );
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

Datum fn_lseg_intersect_point( PG_FUNCTION_ARGS )
{
    LSEG *  a     = NULL;
    LSEG *  b     = NULL;
    Point * isect = NULL;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    a = PG_GETARG_LSEG_P(0);
    b = PG_GETARG_LSEG_P(1);

    if( a == NULL || b == NULL )
    {
        PG_RETURN_NULL();
    }

    isect = line_segment_intersection( a, b );

    if( isect == NULL )
    {
        PG_RETURN_NULL();
    }

    PG_RETURN_POINT_P( isect );
}

Datum fn_lseg_intersect( PG_FUNCTION_ARGS )
{
    LSEG * a = NULL;
    LSEG * b = NULL;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    a = PG_GETARG_LSEG_P(0);
    b = PG_GETARG_LSEG_P(1);

    if( a == NULL || b == NULL )
    {
        PG_RETURN_NULL();
    }

    if( line_segment_intersect( a, b ) )
    {
        PG_RETURN_BOOL( true );
    }

    PG_RETURN_BOOL( false );
}

Datum fn_lseg_distance( PG_FUNCTION_ARGS )
{
    LSEG * a         = NULL;
    LSEG * b         = NULL;
    double distance  = 0.0;

    if( PG_ARGISNULL(0) || PG_ARGISNULL(1) )
    {
        PG_RETURN_NULL();
    }

    a = PG_GETARG_LSEG_P(0);
    b = PG_GETARG_LSEG_P(1);

    if( a == NULL || b == NULL )
    {
        PG_RETURN_NULL();
    }

    distance = line_segment_distance( a, b );

    PG_RETURN_FLOAT8( distance );
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
