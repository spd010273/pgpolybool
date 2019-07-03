/*------------------------------------------------------------------------------
 * convex_hull.c
 *     Implementation of a convex hull algorithm using Graham scan.
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      convex_hull.c
 *
 *------------------------------------------------------------------------------
 */

#include "convex_hull.h"

POLYGON * get_convex_hull( Point ** point_field, unsigned int num_points )
{
    POLYGON *    result    = NULL;
    Point **     stack     = NULL;
    unsigned int stack_top = 0;
    unsigned int size      = 0;
    unsigned int i         = 0;

    if( point_field == NULL || num_points == 0 )
    {
        return NULL;
    }

    stack = ( Point ** ) palloc0( num_points * sizeof( Point * ) );

    if( stack == NULL )
    {
        __oom( "Failed to allocate point stack" );
    }

    sort_points( point_field, num_points, 0, num_points - 1 );

    stack_top = 2;

    for( i = 0; i <= stack_top; i++ )
    {
        stack[i] = point_field[i];
    }

    for( i = stack_top + 1; i < num_points; i++ )
    {
        while(
                get_turn_type(
                    stack[stack_top - 1],
                    stack[stack_top],
                    point_field[i]
                ) == right_turn
             )
        {
            stack_top--;
        }

        stack_top++;
        stack[stack_top] = point_field[i];
    }

    size = offsetof( POLYGON, p ) + ( sizeof( Point ) * ( stack_top + 1 ) );

    result = ( POLYGON * ) palloc0( size );

    if( result == NULL )
    {
        __oom( "Failed to allocate convex hull polygon" );
    }

    result->npts = stack_top + 1;

    for( i = 0; i < result->npts; i++ )
    {
        result->p[i].x = stack[i]->x;
        result->p[i].y = stack[i]->y;
    }


    pfree( stack );
    return result;
}

turn get_turn_type( Point * p0, Point * p1, Point * p2 )
{
    double turn_angle = 0.0;

    if( p0 == NULL || p1 == NULL || p2 == NULL )
    {
        return 0.0;
    }

    turn_angle = ( p1->y - p0->y ) * ( p2->x - p1->x )
               - ( p1->x - p0->x ) * ( p2->y - p1->y );

    if( fabs( turn_angle ) <= DBL_EPSILON )
    {
        return inline_turn;
    }

    if( turn_angle > 0 )
    {
        return right_turn;
    }

    return left_turn;
}

Point * get_lowest_point( Point ** points, unsigned int num_points )
{
    unsigned int i = 0;
    Point * result = NULL;

    if( points == NULL || num_points == 0 )
    {
        return NULL;
    }

    result = points[0];

    for( i = 0; i < num_points; i++ )
    {
        if( points[i]->y < result->y )
        {
            result = points[i];
        }
        else if(
                    ( fabs( points[i]->y - result->y ) < DBL_EPSILON )
                 && points[i]->x < result->x
               )
        {
            result = points[i];
        }
    }

    return result;
}

double get_point_angle( Point * p, Point * lowest_point )
{
    double angle = 0.0;

    if( p == NULL || lowest_point == NULL )
    {
        return 0.0;
    }

    if( p == lowest_point )
    {
        return -1.0;
    }

    angle = acos( ( p->x - lowest_point->x ) / distance( lowest_point, p ) );

    return angle;
}

void sort_points(
    Point **     points,
    unsigned int num_points,
    unsigned int left_bound,
    unsigned int right_bound
)
{
    unsigned int center = 0;

    if( points == NULL )
    {
        return;
    }

    if( left_bound < right_bound )
    {
        center = left_bound + ( ( right_bound - left_bound ) / 2 );
        sort_points( points, num_points, left_bound, center );
        sort_points( points, num_points, center + 1, right_bound );
        merge( points, num_points, left_bound, center, right_bound );
    }

    return;
}

void merge(
    Point ** points,
    unsigned int num_points,
    unsigned int left_bound,
    unsigned int center,
    unsigned int right_bound
)
{
    unsigned int i                = 0;
    unsigned int j                = 0;
    unsigned int k                = 0;
    unsigned int len_partition_0  = 0;
    unsigned int len_partition_1  = 0;
    Point **     temp_partition_0 = NULL;
    Point **     temp_partition_1 = NULL;
    Point *      lowest_point     = NULL;
    double       angle_0          = 0.0;
    double       angle_1          = 0.0;

    if( points == NULL )
    {
        return;
    }

    len_partition_0 = center - left_bound + 1;
    len_partition_1 = right_bound - center;

    temp_partition_0 = ( Point ** ) palloc0(
        len_partition_0 * sizeof( Point * )
    );

    temp_partition_1 = ( Point ** ) palloc0(
        len_partition_1 * sizeof( Point * )
    );

    if( temp_partition_0 == NULL || temp_partition_1 == NULL )
    {
        __oom( "Failed to mergesort convex hull" );
    }

    for( i = 0; i < len_partition_0; i++ )
    {
        temp_partition_0[i] = points[left_bound + i];
    }

    for( i = 0; i < len_partition_1; i++ )
    {
        temp_partition_1[i] = points[center + 1 + i];
    }

    i = 0;
    j = 0;
    k = left_bound;

    lowest_point = get_lowest_point( points, num_points );

    while( i < len_partition_0 && j < len_partition_1 )
    {
        angle_0 = get_point_angle( temp_partition_0[i], lowest_point );
        angle_1 = get_point_angle( temp_partition_1[j], lowest_point );

        if( angle_0 < angle_1 || fabs( angle_0 - angle_1 ) < DBL_EPSILON )
        {
            points[k] = temp_partition_0[i];
            i++;
        }
        else
        {
            points[k] = temp_partition_1[j];
            j++;
        }

        k++;
    }

    while( i < len_partition_0 )
    {
        points[k] = temp_partition_0[i];
        i++;
        k++;
    }

    while( j < len_partition_1 )
    {
        points[k] = temp_partition_1[j];
        j++;
        k++;
    }

    pfree( temp_partition_0 );
    pfree( temp_partition_1 );

    return;
}
