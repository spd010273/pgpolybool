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

/*
 * Preprocessing frontend that handles buffering, scaling and sorting of polys
 * from function input
 * We CANNOT modify the original polygons, as they may be pointers to something
 * in the disk buffer.
 */

/*
 * POLYGON * poly_preprocessing( POLYGON *, bool, Point ** )
 *
 *      Polygon preprocessing function that scales the input polygon by ZOOM_RATE.
 *      This is useful for UNION and INTERSECTION ops, where the polygons may
 *      share a boarder that can impact the output, where the underlying algorithm
 *      requires an intersection of > 0 area to exist in order to produce
 *      meaningful output
 *
 * Arguments:
 *     POLYGON * current_poly: Input polygon to be scaled
 *     bool      scale:        Flag that indicates scaling should take place.
 *     Point **  center:       Pass-by-reference center of the scaled polygon.
 *                             This is used to accurately scale the polygon to
 *                             its original size.
 * Return:
 *     POLYGON * buff_poly:    Scaled / buffered polygon
 * Error Conditions:
 *     Emits error on failure to allocate memory
 */
POLYGON * poly_preprocessing(
    POLYGON * current_poly,
    bool      scale,
    Point **  center
)
{
    POLYGON * buff_poly    = NULL;
    POLYGON * poly         = NULL;
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

    scale = false; //XXX
    if( buff_poly == NULL || (*center) == NULL )
    {
        __oom( "Could not allocate centroid or buffer for polygon" );
    }

    poly = remove_duplicate_and_colinear_points( current_poly, false );

    for( i = 0; i < poly->npts; i++ )
    {
        sum_x += poly->p[i].x;
        sum_y += poly->p[i].y;
        npoints++;
    }

    (*center)->x = sum_x / poly->npts;
    (*center)->y = sum_y / poly->npts;

    for( i = 0; i < poly->npts; i++ )
    {
        if( scale )
        {
            buff_poly->p[i].x = poly->p[i].x * ZOOM_RATE
                              - ( ZOOM_RATE - 1 ) * (*center)->x;
            buff_poly->p[i].y = poly->p[i].y * ZOOM_RATE
                              - ( ZOOM_RATE - 1 ) * (*center)->y;
        }
        else
        {
            buff_poly->p[i].x = poly->p[i].x;
            buff_poly->p[i].y = poly->p[i].y;
        }
    }

    buff_poly->npts     = poly->npts;
    buff_poly->boundbox = poly->boundbox;

    return buff_poly;
}

/*
 * POLYGON ** poly_preprocessing_array(
 *     ArrayType *,
 *     bool,
 *     bool,
 *     Point ***,
 *     unsigned int *
 * )
 *
 *      Polygon preprocessing function that:
 *      - scales the input polygons by ZOOM_RATE.
 *      - sorts the polygons by their centroid distances from element 0
 *      This is useful for UNION and INTERSECTION ops, where the polygons may
 *      share a boarder that can impact the output, where the underlying algorithm
 *      requires an intersection of > 0 area to exist in order to produce
 *      meaningful output.
 *      The sorting is useful for all ops other than XOR and DIFFERENCE.
 *
 * Arguments:
 *     ArrayType * polyarray:   Input polygon array to be sorted / scaled
 *     bool      scale:         Flag that indicates scaling should take place.
 *     bool      sort           Flag that indicates the elements of the array
 *                              should be sorted
 *     Point *** center:        Pass-by-reference array of polygon centroids.
 *     unsigned int * num_poly: Number of polygons allocated in the output
 *                              array.
 * Return:
 *     POLYGON ** buff_poly:    Scaled / buffered polygon array
 * Error Conditions:
 *     Emits error on failure to allocate memory
 *     Emits error on NULL input array element
 */
POLYGON ** poly_preprocessing_array(
    ArrayType *    polyarray,
    bool           sort,
    bool           scale,
    Point ***      centers,
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

    scale = false; //XXX
    if( (*num_poly) <= 1 )
    {
        buff_polys = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );

        if( buff_polys == NULL )
        {
            __oom( "Could not buffer input polygons" );
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
        __oom( "Could not buffer input polygons" );
    }

    palloc_sz  = (*num_poly) * sizeof( Point * );
    (*centers) = ( Point ** ) palloc0( palloc_sz );

    if( (*centers) == NULL )
    {
        __oom( "Could not allocate polygon centroids" );
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
            __oom( "Could not create sorting array for polygons" );
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

/*
 * POLYGON * poly_postprocessing(
 *     POLYGON *,
 *     Point **,
 *     unsigned int *
 *     bool
 * )
 *
 *     This function rescales the output by ZOOM_RATE relative to the previous
 *     centroid of the polygon. This function also removes colinear points from
 *     output polygon.
 *
 * Arguments:
 *     POLYGON * poly:           Input polygon to be de-scaled.
 *     Point ** centers:         Array of centroids. Since the polygon may have
 *                               come from an operation that took > 1 polygon
 *                               as an input, we select the closest centroid as
 *                               the rescaling point.
 *     unsigned int num_centers: Number of centroids in the centers array
 *     bool         scale:       Flag indicating we should rescale the input
 * Output:
 *     POLYGON * poly:           optionally scaled output polygon.
 * Error Conditions:
 *     None.
 */
POLYGON * poly_postprocessing(
    POLYGON *    poly,
    Point **     centers,
    unsigned int num_centers,
    bool         scale
)
{
    Point *      center          = NULL;
    Point *      min_dist_center = NULL;
    double       min_distance    = 0.0;
    double       avg_dist        = 0.0;
    unsigned int i               = 0;
    unsigned int j               = 0;

    if( poly == NULL )
    {
        return NULL;
    }
    scale = false; //XXX
    min_distance = DBL_MAX;

    if( scale )
    {
        for( i = 0; i < num_centers; i++ )
        {
            center = centers[i];
            avg_dist = 0.0;

            for( j = 0; j < poly->npts; j++ )
            {
                avg_dist += distance( center, ( &(poly->p[j]) ) );
            }

            avg_dist = avg_dist / poly->npts;

            if( avg_dist < min_distance )
            {
                min_distance    = avg_dist;
                min_dist_center = center;
            }
        }

        if( min_dist_center != NULL )
        {
            for( i = 0; i < poly->npts; i++ )
            {
                poly->p[i].x = (
                                    poly->p[i].x
                                  + ( ZOOM_RATE - 1 ) * min_dist_center->x
                               ) / ZOOM_RATE;
                poly->p[i].y = (
                                    poly->p[i].y
                                  + ( ZOOM_RATE - 1 ) * min_dist_center->y
                               ) / ZOOM_RATE;
            }
        }
        else
        {
            elog( WARNING, "Failed to de-scale polygon after OP_UNION" );
        }
    }

    if( poly->npts < 3 )
    {
        return poly;
    }

    poly = remove_duplicate_and_colinear_points( poly, true );
    return poly;
}

/*
 *  bool points_colinear(
 *      Point *,
 *      Point *,
 *      Point *
 *  )
 *
 *      Function for testing if three points lie on the same line
 *
 *  Arguments:
 *      Point * p0: First point under test
 *      Point * p1: Second point under test
 *      Point * p2: Third point under test
 *  Result:
 *      bool result: True indicates that the point lie on the same line
 *  Error Conditions:
 *      None
 */
bool points_colinear( Point * p0, Point * p1, Point * p2 )
{
    double area = 0.0;

    if( p0 == NULL || p1 == NULL || p2 == NULL )
    {
        return false;
    }

    area = signed_area_three( p0, p1, p2 );

    if( fabs( area ) <= DBL_EPSILON * PP_DEDUPE_FUDGE_FACTOR )
    {
        return true;
    }

    return false;
}

/*
 * void remove_colinear_point( POLYGON **, Point * )
 *
 *     Removes the specified point from a polygon
 *
 * Arguments:
 *     Polygon ** poly:        Polygon from which the point in being removed.
 *     Point * colinear_point: Point to be removed
 * Return:
 *     None
 * Error Conditions:
 *     Emits error on failure to allocate a new polygon
 */
void remove_colinear_point( POLYGON ** poly, Point * colinear_point )
{
    POLYGON *    new_poly = NULL;
    unsigned int i        = 0;
    unsigned int offset   = 0;

    if( (*poly) == NULL || colinear_point == NULL )
    {
        return;
    }

    new_poly = ( POLYGON * ) palloc0(
        offsetof( POLYGON, p )
      + ( sizeof( Point ) * ( (*poly)->npts - 1 ) )
    );

    if( new_poly == NULL )
    {
        __oom( "Could not create new polygon" );
    }

    new_poly->npts = (*poly)->npts - 1;

    for( i = 0; i < (*poly)->npts - 1; i++ )
    {
        if( points_equal( (&((*poly)->p[i])), colinear_point ) )
        {
            offset = 1;
        }

        new_poly->p[i].x = (*poly)->p[i + offset].x;
        new_poly->p[i].y = (*poly)->p[i + offset].y;
    }

    colinear_point->x = 0;
    colinear_point->y = 0;

    pfree( (*poly) );

    (*poly) = new_poly;
    return;
}

POLYGON * remove_duplicate_and_colinear_points( POLYGON * poly, bool free_input )
{
    unsigned int i                     = 0;
    unsigned int j                     = 0;
    unsigned int k                     = 0;
    unsigned int final_size            = 0;
    POLYGON *    temp_poly             = NULL; // Intermediate Polygon
    POLYGON *    result_poly           = NULL;
    Point *      p0                    = NULL;
    Point *      p1                    = NULL;
    Point *      p2                    = NULL;
    Point        max                   = {0.0};
    Point        min                   = {0.0};
    bool         colinear_points_found = false;
    bool *       skiplist              = NULL;

    if( poly == NULL )
    {
        return NULL;
    }

    final_size = poly->npts;

    temp_poly = ( POLYGON * ) palloc0(
        offsetof( POLYGON, p )
      + sizeof( Point ) * ( poly->npts )
    );

    // Pad out the skiplist to the next multiple of sizeof( void * )
    skiplist = ( bool * ) palloc0(
        ( poly->npts + 7 ) & ( -sizeof( void * ) )
    );

    if( temp_poly == NULL )
    {
        __oom(
                "Could not create buffer polygon for"\
                " duplicate / colinear point removal"
        );
    }

    if( skiplist == NULL )
    {
        __oom(
            "Failed to generate skiplist for array distinct"
        );
    }

    for( i = 0; i < poly->npts; i++ )
    {
        for( j = i + 1; j < poly->npts; j++ )
        {
            if(
                   ( fabs( poly->p[i].x - poly->p[j].x ) <= DBL_EPSILON * PP_DEDUPE_FUDGE_FACTOR )
                && ( fabs( poly->p[i].y - poly->p[j].y ) <= DBL_EPSILON * PP_DEDUPE_FUDGE_FACTOR )
              )
            {
                skiplist[j] = true;
            }
        }

        if( !skiplist[i]  )
        {
            temp_poly->p[k].x = poly->p[i].x;
            temp_poly->p[k].y = poly->p[i].y;
            k++;
        }
    }

    pfree( skiplist );
    final_size      = k;
    temp_poly->npts = final_size;

    // Scan for colinear points
    colinear_points_found = true;

    while( colinear_points_found )
    {
        colinear_points_found = false;

        if( temp_poly->npts < 3 )
        {
            break;
        }

        for( i = 0; i < temp_poly->npts; i++ )
        {
            p0 = &(temp_poly->p[i]);
            p1 = &(temp_poly->p[( i + 1 ) % temp_poly->npts]);
            p2 = &(temp_poly->p[( i + 2 ) % temp_poly->npts]);

            colinear_points_found = points_colinear( p0, p1, p2 );

            if( colinear_points_found )
            {
                remove_colinear_point( &temp_poly, p1 );
                final_size--;
                break;
            }

            p0 = NULL;
            p1 = NULL;
            p2 = NULL;
        }
    }

    result_poly = ( POLYGON * ) palloc0(
        offsetof( POLYGON, p )
      + sizeof( Point ) * final_size
    );

    if( result_poly == NULL )
    {
        pfree( temp_poly );
        __oom( "Could not create output polygon for point deduplication result" );
    }

    max.x = -DBL_MAX;
    max.y = -DBL_MAX;
    min.x = DBL_MAX;
    min.y = DBL_MAX;
    // Copy poly into output and set bounding box in one go
    for( i = 0; i < final_size; i++ )
    {
        if( temp_poly->p[i].x > max.x )
        {
            max.x = temp_poly->p[i].x;
        }
        else if( temp_poly->p[i].x < min.x )
        {
            min.x = temp_poly->p[i].x;
        }

        if( temp_poly->p[i].y > max.y )
        {
            max.y = temp_poly->p[i].y;
        }
        else if( temp_poly->p[i].y < min.y )
        {
            min.y = temp_poly->p[i].y;
        }

        result_poly->p[i].x = temp_poly->p[i].x;
        result_poly->p[i].y = temp_poly->p[i].y;
    }

    pfree( temp_poly );
    result_poly->npts = final_size;
    result_poly->boundbox.high.y = max.y;
    result_poly->boundbox.high.x = max.x;
    result_poly->boundbox.low.y = min.y;
    result_poly->boundbox.low.x = min.x;

    if( final_size != poly->npts && free_input )
    {
        pfree( poly );
    }

    SET_VARSIZE(
        result_poly,
        offsetof( POLYGON, p )
      + sizeof( Point ) * final_size
    );
    return result_poly;
}
