/*------------------------------------------------------------------------------
 * ombb.c
 *      Oriented minimum bounding box. Implementation is in O n^2 time and can
 *      certainly be improved upon
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      ombb.c
 *
 *------------------------------------------------------------------------------
 */

/*
 * Use the property of OMBBs that one edge is colinear with an edge of the input convex hull
 * We also take turns rotating the hull and finding the axis-oriented bounding box, since this
 * is a muuuch easier process
 * https://github.com/cansik/LongLiveTheSquare
 */

#include "ombb.h"

Point ** get_ombb(
    POLYGON * p,
    double *  o_len_o,
    double *  o_len_p,
    double *  o_theta,
    bool      discard_points
)
{
    Point **               result           = NULL;
    Point                  center           = {0};
    LSEG *                 edge_list        = NULL;
    struct ombb_solution * solution_list    = NULL;
    unsigned int           i                = 0;
    unsigned int           min_area_ind     = 0;
    double                 rotation_angle   = 0.0;
    double                 cumulative_angle = 0.0;
    double                 area             = 0.0;
    double                 height           = 0.0;
    double                 width            = 0.0;

    if( p == NULL )
    {
        return NULL;
    }

    // Allocate edge list and result
    edge_list     = ( LSEG * ) palloc0( sizeof( LSEG ) * p->npts );
    result        = ( Point ** ) palloc0( sizeof( Point * ) * 4 );
    solution_list = ( struct ombb_solution * ) palloc0(
        sizeof( struct ombb_solution ) * p->npts
    );

    if( result == NULL || edge_list == NULL || solution_list == NULL )
    {
        __oom( "Failed to allocate edge list for ombb calculation" );
    }

    for( i = 0; i < 4; i++ )
    {
        result[i] = ( Point * ) palloc0( sizeof( Point ) );

        if( result[i] == NULL )
        {
            __oom( "Failed to allocate point result" );
        }
    }

    for( i = 0; i < p->npts; i++ )
    {
        center.x           += p->p[i].x;
        center.y           += p->p[i].y;
        edge_list[i].p[0].x = p->p[i].x;
        edge_list[i].p[0].y = p->p[i].y;
        edge_list[i].p[1].x = p->p[(i + 1)%p->npts].x;
        edge_list[i].p[1].y = p->p[(i + 1)%p->npts].y;
    }

    center.x = center.x / p->npts;
    center.y = center.y / p->npts;

    for( i = 0; i < p->npts; i++ )
    {
        rotation_angle = -atan(
            ( edge_list[i].p[1].y - edge_list[i].p[0].y )
          / ( edge_list[i].p[1].x - edge_list[i].p[0].x )
        );

        cumulative_angle += rotation_angle;
        rotate_edge_list( edge_list, p->npts, &center, rotation_angle );
        // Pick the ith edge and rotate it to be parallel w/ the x-axis,
        // then compute the axis-oriented bounding box for that polygon.
        // Everything is done about the centroid
        get_aa_bounding_box( edge_list, p->npts, &height, &width, &area );
        solution_list[i].angle  = cumulative_angle;
        solution_list[i].area   = area;
        solution_list[i].height = height;
        solution_list[i].width  = width;
    }

    area = DBL_MAX;
    for( i = 0; i < p->npts; i++ )
    {
        if( solution_list[i].area < area )
        {
            area = solution_list[i].area;
            min_area_ind = i;
        }
    }

    result[0]->x = center.x - ( width / 2 );
    result[0]->y = center.y - ( height / 2 );
    result[1]->x = center.x + ( width / 2 );
    result[1]->y = center.y - ( height / 2 );
    result[2]->x = center.x + ( width / 2 );
    result[2]->y = center.y + ( height / 2 );
    result[3]->x = center.x - ( width / 2 );
    result[3]->y = center.y + ( height / 2 );

    // Perform final rotation
    rotation_angle = solution_list[min_area_ind].angle;

    for( i = 0; i < 4; i++ )
    {
        rotate_point_about_center( result[i], &center, rotation_angle );
    }

    if( o_len_o != NULL )
    {
        *o_len_o = solution_list[min_area_ind].height;
    }

    if( o_len_p != NULL )
    {
        *o_len_p = solution_list[min_area_ind].width;
    }

    if( o_theta != NULL )
    {
        *o_theta = solution_list[min_area_ind].angle;
    }

    pfree( edge_list );
    pfree( solution_list );

    return result;
}

void get_aa_bounding_box(
    LSEG *       edge_list,
    unsigned int num_edges,
    double *     height,
    double *     width,
    double *     area
)
{
    unsigned int i = 0;
    Point max = {0};
    Point min = {0};

    max.x = -DBL_MAX;
    max.y = -DBL_MAX;
    min.x = DBL_MAX;
    min.y = DBL_MAX;

    if( edge_list == NULL || height == NULL || width == NULL || area == NULL )
    {
        return;
    }

    for( i = 0; i < num_edges; i++ )
    {
        if( edge_list[i].p[0].x > max.x )
        {
            max.x = edge_list[i].p[0].x;
        }
        else if( edge_list[i].p[0].x < min.x )
        {
            min.x = edge_list[i].p[0].x;
        }

        if( edge_list[i].p[0].y > max.y )
        {
            max.y = edge_list[i].p[0].y;
        }
        else if( edge_list[i].p[0].y < min.y )
        {
            min.y = edge_list[i].p[0].y;
        }
    }

    *width  = max.y - min.y;
    *height = max.x - min.x;
    *area   = ( max.y - min.y ) * ( max.x - min.x );

    return;
}

void rotate_edge_list(
    LSEG *       edge_list,
    unsigned int num_edges,
    Point *      center,
    double       angle
)
{
    unsigned int i = 0;

    if( edge_list == NULL || center == NULL )
    {
        return;
    }

    for( i = 0; i < num_edges; i++ )
    {
        rotate_point_about_center( &(edge_list[i].p[0]), center, angle );
        rotate_point_about_center( &(edge_list[i].p[1]), center, angle );
    }

    return;
}

void rotate_point_about_center( Point * p, Point * center, double angle )
{
    Point temp = {0};

    if( p == NULL || center == NULL )
    {
        return;
    }

    temp.x = p->x;
    temp.y = p->y;

    p->x = ( temp.x * cos( angle ) )
         - ( temp.y * sin( angle ) )
         + (
               center->x
             - (
                   center->x * cos( angle )
                 - center->y * sin( angle )
               )
           );

    p->y = ( temp.x * sin( angle ) )
         + ( temp.y * cos( angle ) )
         + (
               center->y
             - (
                   center->x * sin( angle )
                 + center->y * cos( angle )
               )
           );

    return;
}
