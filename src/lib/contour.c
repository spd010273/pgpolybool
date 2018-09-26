#include "contour.h"

void contour_bounding_box( struct contour * c, Point * min, Point * max )
{
    int i = 0;
    double max_x = 0;
    double max_y = 0;
    double min_x = 0;
    double min_y = 0;

    if( c == NULL )
    {
        return;
    }

    max_x = -DBL_MAX;
    max_y = -DBL_MAX;
    min_x = DBL_MAX;
    min_y = DBL_MAX;

    for( i = 0; i < c->num_points; i++ )
    {
        if( c->points[i]->x < min_x )
        {
            min_x = c->points[i]->x;
        }

        if( c->points[i]->x > max_x )
        {
            max_x = c->points[i]->x;
        }

        if( c->points[i]->y < min_y )
        {
            min_y = c->points[i]->y;
        }

        if( c->points[i]->y > max_y )
        {
            max_y = c->points[i]->y;
        }
    }

    if( min == NULL )
    {
        min = ( Point * ) palloc0( sizeof( Point ) );
    }

    if( max == NULL )
    {
        max = ( Point * ) palloc0( sizeof( Point ) );
    }

    min->x = min_x;
    min->y = min_y;
    max->x = max_x;
    max->y = max_y;

    return;
}

bool contour_counterclockwise( struct contour * c )
{
    double area = 0.0;
    int i = 0;
    if( c == NULL )
    {
        return false;
    }

    if( c->_precomputed_cc )
    {
        return c->_cc;
    }

    c->_precomputed_cc = true;

    for( i = 0; i < c->num_points - 1; i++ )
    {
        area += c->points[i]->x * c->points[i+1]->y - c->points[i+1]->x * c->points[i]->y;
    }

    area += c->points[c->num_points - 1]->x * c->points[0]->y
          + c->points[0]->x * c->points[c->num_points - 1]->y;

    if( fabs( area ) <= DBL_EPSILON )
    {
        c->_cc = false;
    }
    else
    {
        c->_cc = true;
    }

    return c->_cc;
}

bool contour_clockwise( struct contour * c )
{
    if( c == NULL )
    {
        return false;
    }

    return !contour_counterclockwise( c );
}

void contour_set_clockwise( struct contour * c )
{
    if( c == NULL )
    {
        return;
    }

    if( contour_counterclockwise( c ) )
    {
        contour_change_orientation( c );
    }

    return;
}

void contour_set_counterclockwise( struct contour * c )
{
    if( c == NULL )
    {
        return;
    }

    if( !( contour_counterclockwise( c ) ) )
    {
        contour_change_orientation( c );
    }

    return;
}

void contour_change_orientation( struct contour * c )
{
    int start_i  = 0;
    int end_i    = 0;
    int j        = 0;
    Point * temp = NULL;
 
    if( c == NULL )
    {
        return;
    }
    
    end_i = c->num_points;
   
    while( start_i < end_i )
    {
        for( j = 0; j < c->num_holes; j++ )
        {
            if( c->holes[j] == start_i )
            {
                c->holes[j] = end_i;
            }
            else if( c->holes[j] == end_i )
            {
                c->holes[j] = start_i;
            }
        }

        temp               = c->points[start_i];
        c->points[start_i] = c->points[end_i];
        c->points[end_i]   = temp;

        start_i++;
        end_i--;
    }

    c->_cc = !( c->_cc );
    return;
}

void contour_erase_point( struct contour * c, int i )
{
    int j = 0;
    int h = 0;
    int c_i = 0;
    int * temp_holes = NULL;
    Point ** temp_points = NULL;

    if( c == NULL )
    {
        return;
    }

    if( i >= c->num_points )
    {
        return;
    }

    pfree( c->points[i] );
    c->points[i] = NULL;

    temp_points = ( Point ** ) palloc0( sizeof( Point * ) * ( c->num_points - 1 ) );
    for( j = 0; j < c->num_points; j++ )
    {
        if( c->points[j] != NULL )
        {
            temp_points[c_i] = c->points[j];
            c_i++;
        }
    }
    
    c->num_points = c->num_points - 1;
    pfree( c->points );
    c->points = temp_points;

    // Erase hole reference if present
    for( j = 0; j < c->num_holes; j++ )
    {
        if( c->holes[j] == i )
        {
            temp_holes = ( int * ) palloc0( sizeof( int ) * c->num_holes - 1 );
            
            for( h = 0; h < c->num_holes; h++ )
            {
                if( h != j )
                {
                    temp_holes[h] = c->holes[h];
                }
            }

            pfree( c->holes );
            c->holes = temp_holes;
            c->num_holes = c->num_holes - 1;
        }
    }

    return;
}

void contour_add_hole( struct contour * c, int index )
{
    if( c == NULL )
    {
        return;
    }

    if( c->holes == NULL )
    {
        if( c->num_holes != 0 )
        {
            return;
        }

        c->holes = ( int * ) palloc0( sizeof( int ) );
        c->num_holes = 1;
        c->holes[0] = index;
    }
    else
    {
        c->holes = ( int * ) repalloc( c->holes, c->num_holes + 1 );
        c->holes[c->num_holes] = index;
        c->num_holes = c->num_holes + 1;
    }

    return;
}

void contour_set_external( struct contour * c, bool ext )
{
    if( c == NULL )
    {
        return;
    }

    c->_external = ext;
}

void contour_add_point( struct contour * c, Point * p )
{
    if( c == NULL )
    {
        return;
    }

    if( c->points == NULL )
    {
        if( c->num_points != 0 )
        {
            return;
        }
       
        c->points = ( Point ** ) palloc0( sizeof( Point * ) );
        c->num_points = 1;
        c->points[0] = p;
    }
    else
    {
        c->points = ( Point ** ) repalloc( c->points, sizeof( Point * ) * c->num_points + 1 );
        c->points[c->num_points] = p;
        c->num_points = c->num_points + 1;
    }

    return;
}

struct segment * contour_get_segment( struct contour * c, int index )
{
    struct segment * s = NULL;

    if( c == NULL )
    {
        return NULL;
    }

    s = new_segment();

    if( index == ( c->num_points - 1 ) )
    {
        segment_set_begin( s, c->points[c->num_points - 1] );
        segment_set_end( s, c->points[0] ); 
    }
    else
    {
        segment_set_begin( s, c->points[index] );
        segment_set_end( s, c->points[index + 1] );
    }

    return s;
}

struct contour * new_contour( void )
{
    struct contour * c = NULL;

    c = ( struct contour * ) palloc0( sizeof( struct contour ) );
    c->num_points = 0;
    c->num_holes = 0;
    c->_external = false;
    c->_precomputed_cc = false;
    c->_cc = false;

    return c;
}

void free_contour( struct contour * c )
{
    if( c == NULL )
    {
        return;
    }

    if( c->points != NULL )
    {
        pfree( c->points );
    }

    if( c->holes != NULL )
    {
        pfree( c->holes );
    }

    pfree( c );

    return;
}

double contour_area( struct contour * c )
{
    double area = 0.0;
    int i = 0;

    if( c == NULL )
    {
        return area;
    }

    for( i = 0; i < c->num_points; i++ )
    {
        if( i == c->num_points - 1 )
        {
            area += c->points[i]->x * c->points[0]->y
                  - c->points[0]->x * c->points[i]->y;
        }
        else
        {
            area += c->points[i]->x * c->points[i + 1]->y
                  - c->points[i + 1]->x * c->points[i]->y;
        }
    }

    area = area / 2;
    return area;
}
