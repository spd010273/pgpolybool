/*------------------------------------------------------------------------------
 * poly_funcs.h
 *      Helper functions of pgpolybool polygon functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      poly_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#include "poly_funcs.h"

/*
 * void set_polygon_boundbox( POLYGON * )
 *
 *     Sets the polygon's boundingbox based on the polygons points
 *
 * Arguments:
 *     POLYGON * p: The polygon we are finding a bounding box for
 * Return:
 *     None
 * Error Conditions:
 *     None
 */
void set_polygon_boundbox( POLYGON * p )
{
    double       max_x = -DBL_MAX;
    double       max_y = -DBL_MAX;
    double       min_x = DBL_MAX;
    double       min_y = DBL_MAX;
    unsigned int i     = 0;

    if( p == NULL )
    {
        return;
    }

    for( i = 0; i < p->npts; i++ )
    {
        if( p->p[i].x > max_x )
        {
            max_x = p->p[i].x;
        }
        else if( p->p[i].x < min_x )
        {
            min_x = p->p[i].x;
        }

        if( p->p[i].y > max_y )
        {
            max_y = p->p[i].y;
        }
        else if( p->p[i].y < min_y )
        {
            min_y = p->p[i].y;
        }
    }

    p->boundbox.high.x = max_x;
    p->boundbox.high.y = max_y;
    p->boundbox.low.x  = min_x;
    p->boundbox.low.y  = min_y;

    return;
}

/*
 * double get_polygon_area( POLYGON * )
 *
 *     Returns the area of a given polygon using Gauss' area formula
 *
 * Arguments:
 *     Polygon * p: The polygon for which we are finding the area
 * Return
 *     double area: The area of the polygon
 * Error Conditions:
 *     None
 */
double get_polygon_area( POLYGON * p )
{
    double       p_area = 0.0;
    double       n_area = 0.0;
    unsigned int i      = 0;

    if( p == NULL )
    {
        return 0.0;
    }

    // Points and lines have 0 area
    if( p->npts <= 2 )
    {
        return 0.0;
    }

    // Use Gauss' Area formula (shoelace formula)
    for( i = 0; i < p->npts - 1; i++ )
    {
        p_area += p->p[i].x * p->p[i + 1].y;
        n_area += p->p[i + 1].x * p->p[i].y;
    }

    p_area += p->p[p->npts - 1].x * p->p[1].y;
    n_area += p->p[1].x * p->p[p->npts - 1].y;

    p_area = 0.5 * fabs( p_area - n_area );

    if( p_area < DBL_EPSILON )
    {
        return 0.0;
    }

    return p_area;
}

void rotate_polygon( POLYGON * p, double radians )
{
    unsigned int i        = 0;
    Point        center   = {0.0};
    double       min_x    = DBL_MAX;
    double       max_x    = -DBL_MAX;
    double       min_y    = DBL_MAX;
    double       max_y    = -DBL_MAX;
    double       x        = 0.0;
    double       y        = 0.0;

    if( p == NULL )
    {
        return;
    }

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

    if( p == NULL )
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
        return;
    }

    // Get center from boundingbox
    center.x = ( p->boundbox.high.x + p->boundbox.low.x ) / 2;
    center.y = ( p->boundbox.high.y + p->boundbox.low.y ) / 2;

    for( i = 0; i < p->npts; i++ )
    {
        x = p->p[i].x;
        y = p->p[i].y;

        p->p[i].x = ( x * cos( radians ) )
                  - ( y * sin( radians ) )
                  + (
                       center.x
                     - (
                           center.x * cos( radians )
                         - center.y * sin( radians )
                       )
                    );
        p->p[i].y = ( x * sin( radians ) )
                  + ( y * cos( radians ) )
                  + (
                       center.y
                     - (
                           center.x * sin( radians )
                         + center.y * cos( radians )
                       )
                    );

        // Generate new bounding box inline
        max_x = ( p->p[i].x > max_x ) ? p->p[i].x : max_x;
        min_x = ( p->p[i].x < min_x ) ? p->p[i].x : min_x;
        max_y = ( p->p[i].y > max_y ) ? p->p[i].y : max_y;
        min_y = ( p->p[i].y < min_y ) ? p->p[i].y : min_y;
    }

    p->boundbox.high.x = max_x;
    p->boundbox.high.y = max_y;
    p->boundbox.low.x  = min_x;
    p->boundbox.low.y  = min_y;

    return;
}

Point ** get_polygon_points( POLYGON * p )
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
            for( j = 0; j < i; j-- )
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

/*
 * void dump_polygon( POLYGON * )
 *
 *     Dump the POLYGON struct for inspection
 *
 * Arguments:
 *     POLYGON * p: The polygon to be inspected
 * Return:
 *     None
 * Error Conditions:
 *     None
 */
#ifdef DEBUG
void dump_polygon( POLYGON * p )
{
    unsigned int i = 0;

    if( p == NULL )
    {
        return;
    }

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
