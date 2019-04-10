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

    SET_VARSIZE(
        result,
        offsetof( POLYGON, p )
      + sizeof( Point ) * 4
    );

    return result;
}

// Similar to the helpers in lseg_funcs, we use a quadratic to find points
POLYGON * line_segment_to_polygon( LSEG * segment, double width )
{
    POLYGON * result  = NULL;
    double a          = 0.0;
    double b_0        = 0.0;
    double c_0        = 0.0;
    double b_1        = 0.0;
    double c_1        = 0.0;
    double curr_slope = 0.0;
    double targ_slope = 0.0;
    double y_int_0    = 0.0;
    double y_int_1    = 0.0;

    if( segment == NULL )
    {
        return NULL;
    }

    result = ( POLYGON * ) palloc0(
        offsetof( POLYGON, p )
      + sizeof( Point ) * 4
    );

    if( result == NULL )
    {
        return NULL;
    }

    if( fabs( segment->p[0].y - segment->p[1].y ) < DBL_EPSILON )
    {
        // Slope is 0, line is horizontal
        result->p[0].x = segment->p[0].x;
        result->p[1].x = segment->p[1].x;
        result->p[2].x = segment->p[1].x;
        result->p[3].x = segment->p[0].x;
        result->p[0].y = segment->p[0].y + ( width / 2 );
        result->p[1].y = segment->p[1].y + ( width / 2 );
        result->p[2].y = segment->p[1].y - ( width / 2 );
        result->p[3].y = segment->p[0].y - ( width / 2 );
    }
    else if( fabs( segment->p[0].x - segment->p[1].x ) < DBL_EPSILON )
    {
        // Slope is infinite, line is horizontal
        result->p[0].y = segment->p[0].y;
        result->p[1].y = segment->p[1].y;
        result->p[2].y = segment->p[1].y;
        result->p[3].y = segment->p[0].y;
        result->p[0].x = segment->p[0].x + ( width / 2 );
        result->p[1].x = segment->p[1].x + ( width / 2 );
        result->p[2].x = segment->p[1].x - ( width / 2 );
        result->p[3].x = segment->p[0].x - ( width / 2 );
    }
    else
    {
        // Solve for y = mx + b for our current line (we just need m)
        curr_slope = ( segment->p[0].y - segment->p[1].y )
                   / ( segment->p[0].x - segment->p[1].x );
        // Invert && negate slope, solve for y=mx+b for two lines orthogonal to
        // the endpoints of our input
        targ_slope = -1.0 / curr_slope;
        y_int_0    = segment->p[0].y - ( segment->p[0].x * targ_slope );
        y_int_1    = segment->p[1].y - ( segment->p[1].x * targ_slope );

        a   = 1.0 + ( targ_slope * targ_slope );

        b_0 = -1.0 * a * 2 * segment->p[0].x;
        b_1 = -1.0 * a * 2 * segment->p[1].x;

        c_0 = a * segment->p[0].x * segment->p[0].x - width;
        c_1 = a * segment->p[1].x * segment->p[1].x - width;

        if(
               pow( b_0, 2 ) < ( 4 * a * c_0 )
            || pow( b_1, 2 ) < ( 4 * a * c_1 )
          )
        {
            elog(
                DEBUG1,
                "Solution(s) for quadratic:\na=%f\nb_0=%f\nc_0=%f\nb_1=%f\nc_1=%f\nare degenerate!",
                a,
                b_0,
                c_0,
                b_1,
                c_1
            );
            pfree( result );
            return NULL;
        }

        result->p[0].x = ( - b_0 + sqrt( pow( b_0, 2 ) - ( 4 * a * c_0 ) ) ) / ( 2 * a );
        result->p[1].x = ( - b_1 + sqrt( pow( b_1, 2 ) - ( 4 * a * c_1 ) ) ) / ( 2 * a );
        result->p[2].x = ( - b_1 - sqrt( pow( b_1, 2 ) - ( 4 * a * c_1 ) ) ) / ( 2 * a );
        result->p[3].x = ( - b_0 - sqrt( pow( b_0, 2 ) - ( 4 * a * c_0 ) ) ) / ( 2 * a );
        result->p[0].y = targ_slope * result->p[0].x + y_int_0;
        result->p[1].y = targ_slope * result->p[1].x + y_int_1;
        result->p[2].y = targ_slope * result->p[2].x + y_int_1;
        result->p[3].y = targ_slope * result->p[3].x + y_int_0;
    }

    result->npts = 4;
    set_polygon_boundbox( result );

    SET_VARSIZE(
        result,
        offsetof( POLYGON, p )
      + sizeof( Point ) * 4
    );

    return result;
}

/*
 * POLYGON * polygon_from_points( Point **, unsigned int )
 *
 *     Generate a polygon struct from the provided points, This naively
 *     generates the polygon with the points in the same order as they
 *     appear in the input array.
 *
 * Arguments:
 *     Point ** array: The points to add to the polygon
 *     unsigned int i: number of points in the array
 * Return:
 *     POLYGON * result: Resulting polygon
 * Error Conditions:
 *     Returns NULL on failure to allocate new POLYGON
 *     Returns NULL on NULL input
 */
POLYGON * polygon_from_points( Point ** points, unsigned int num_points )
{
    POLYGON *    p = NULL;
    unsigned int i = 0;
    unsigned int size = 0;

    if( points == NULL )
    {
        return NULL;
    }

    if( size == 0 )
    {
        return NULL;
    }

    size = offsetof( POLYGON, p )
         + ( sizeof( Point ) * num_points );
    p = ( POLYGON * ) palloc( size );

    if( p == NULL )
    {
        return NULL;
    }

    for( i = 0; i < num_points; i++ )
    {
        if( points[i] == NULL )
        {
            return NULL;
        }

        p->p[i].x = points[i]->x;
        p->p[i].y = points[i]->y;
    }

    p->npts = num_points;

    set_polygon_boundbox( p );

    SET_VARSIZE( p, size );

    return p;
}

POLYGON * lseg_to_polygon( LSEG * line_segment )
{
    POLYGON *    result = NULL;
    unsigned int size   = 0;
    Point        center = {0};
    double       Um     = 0.0;
    double       Ub     = 0.0;
    double       L      = 0.0;
    double       a      = 0.0;
    double       b      = 0.0;
    double       c      = 0.0;

    if( line_segment == NULL )
    {
        return NULL;
    }

    size   = offsetof( POLYGON, p )
           + sizeof( Point ) * 4;
    result = ( POLYGON * ) palloc0( size );

    if( result == NULL )
    {
        return NULL;
    }

    result->npts   = 4;
    result->p[0].x = line_segment->p[0].x;
    result->p[0].y = line_segment->p[0].y;
    result->p[2].x = line_segment->p[1].x;
    result->p[2].y = line_segment->p[1].y;

    center.x = ( line_segment->p[0].x - line_segment->p[1].x ) / 2.0;
    center.y = ( line_segment->p[0].y - line_segment->p[1].y ) / 2.0;

    L = sqrt(
        pow( line_segment->p[0].x - center.x, 2 )
      + pow( line_segment->p[0].y - center.y, 2 )
    );

    Um = -1.0
       / (
            ( line_segment->p[0].y - line_segment->p[1].y )
          / ( line_segment->p[0].x - line_segment->p[1].x )
         );
    Ub = center.x * Um - center.y;
    a  = pow( Um, 2 ) + 1.0;
    b  = 2.0 * Um * ( Ub - center.y );
    c  = pow( Ub - center.y, 2 ) + pow( center.x, 2 ) - pow( L, 2 );

    if( ( b * b ) < ( 4 * a * c ) )
    {
        elog( DEBUG1, "Solution for quadratic a=%f, b=%f, c=%f is degenerate", a, b, c );

        pfree( result );
        return NULL;
    }

    result->p[1].x = ( -b + sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
    result->p[3].x = ( -b - sqrt( pow( b, 2 ) - ( 4 * a * c ) ) ) / ( 2 * a );
    result->p[1].y = Um * result->p[1].x + Ub;
    result->p[3].y = Um * result->p[3].x + Ub;

    set_polygon_boundbox( result );
    SET_VARSIZE( result, size );
    return result;
}

// Cheat, use an lseg that is colinear with the LINE but centered about the
// y-intercept with a legnth of sqrt(2) (output's sides will have length 1)
POLYGON * line_to_polygon( LINE * line )
{
    POLYGON *    result = NULL;
    LSEG         lseg   = {{{0.0}}};
    double       slope  = 0.0;
    double       y_int  = 0.0;

    if( line == NULL )
    {
        return NULL;
    }

    /* LINE is expressed as Ax+By+C=0,
     * where in y=mx+b terms,
     * m = - A / B
     * b = - C / B
     *
     * since each point on our lseg is sqrt(2)/2 away from the y-intercept,
     * our systems of equations:
     * sqrt(2)/2 = sqrt( x**2 + ( y - b )**2 ) -- from length equation
     *
     * simplifies to:
     * .5 = (( A / B )**2 + 1) * x**2
     * or x = +/- ( 0.5 ) / ( ( A/B )**2 + 1 )
     */
    slope = - line->A / line->B;
    y_int = - line->C / line->C;

    lseg.p[0].x = -0.5 / ( pow( slope, 2 ) + 1 );
    lseg.p[1].x = 0.5 / ( pow( slope, 2 ) + 1 );
    lseg.p[0].y = slope * lseg.p[0].x + y_int;
    lseg.p[1].y = slope * lseg.p[1].x + y_int;

    result = lseg_to_polygon( &lseg );
    return result;
}

POLYGON * point_to_polygon( Point * point )
{
    POLYGON * result  = NULL;
    double angle      = 0.0;
    double angle_step = 0.0;
    unsigned int i    = 0;
    unsigned int size = 0;

    if( point == NULL )
    {
        return NULL;
    }

    size = offsetof( POLYGON, p )
         + sizeof( Point ) * DEFAULT_CIRCLE_POLY_POINTS;

    result = ( POLYGON * ) palloc0( size );

    if( result == NULL )
    {
        return NULL;
    }

    result->npts = DEFAULT_CIRCLE_POLY_POINTS;
    angle_step   = ( 2.0 * PI ) / DEFAULT_CIRCLE_POLY_POINTS;

    for( i = 0; i < DEFAULT_CIRCLE_POLY_POINTS; i++ )
    {
        angle = i * angle_step;
        result->p[i].x = point->x - ( DEFAULT_CIRCLE_POLY_RADIUS * cos( angle ) );
        result->p[i].y = point->y + ( DEFAULT_CIRCLE_POLY_RADIUS * sin( angle ) );
    }

    set_polygon_boundbox( result );
    SET_VARSIZE( result, size );
    return result;
}

LSEG ** get_polygon_lsegs( POLYGON * poly )
{
    LSEG **      result = NULL;
    unsigned int i      = 0;
    unsigned int next_i = 0;

    if( poly == NULL )
    {
        return NULL;
    }

    result = ( LSEG ** ) palloc0( sizeof( LSEG * ) * poly->npts );

    if( result == NULL )
    {
        return NULL;
    }

    for( i = 0; i < poly->npts; i++ )
    {
        result[i] = ( LSEG * ) palloc0( sizeof( LSEG ) );

        if( result[i] == NULL )
        {
            for( next_i = 0; next_i < i; next_i++ )
            {
                pfree( result[i] );
            }

            pfree( result );
            return NULL;
        }

        if( i == poly->npts - 1 )
        {
            next_i = 0;
        }
        else
        {
            next_i = i + 1;
        }

        result[i]->p[0].x = poly->p[i].x;
        result[i]->p[0].y = poly->p[i].y;
        result[i]->p[1].x = poly->p[next_i].x;
        result[i]->p[1].y = poly->p[next_i].y;
    }

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
