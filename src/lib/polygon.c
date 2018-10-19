#include "polygon.h"

int polygon_num_points( struct polygon * p )
{
    int i = 0;
    int count = 0;

    if( p == NULL )
    {
        return 0;
    }

    for( i = 0; i < p->num_contours; i++ )
    {
        if( p->contours[i] != NULL )
        {
            count += p->contours[i]->num_points;
        }
    }

    return count;
}

void polygon_boundingbox( struct polygon * p, Point * min, Point * max )
{
    double min_x = 0;
    double min_y = 0;
    double max_x = 0;
    double max_y = 0;
    int i = 0;
    Point min_temp = {0.0};
    Point max_temp = {0.0};

    min_x = DBL_MAX;
    min_y = DBL_MAX;
    max_x = -DBL_MAX;
    max_y = -DBL_MAX;

    if( p == NULL )
    {
        return;
    }

    for( i = 0; i < p->num_contours; i++ )
    {
        if( p->contours[i] == NULL )
        {
            return;
        }

        contour_bounding_box( p->contours[i], &min_temp, &max_temp );

        if( _fp_lt( min_temp.x, min_x ) )
        {
            min_x = min_temp.x;
        }

        if( _fp_lt( min_temp.y, min_y ) )
        {
            min_y = min_temp.y;
        }

        if( _fp_gt( max_temp.x, max_x ) )
        {
            max_x = max_temp.x;
        }

        if( _fp_gt( max_temp.y, max_y ) )
        {
            max_y = max_temp.y;
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

void polygon_move( struct polygon * p, double x, double y )
{
    int i = 0;
    int j = 0;

    if( p == NULL )
    {
        return;
    }

    for( i = 0; i < p->num_contours; i++ )
    {
        if( p->contours[i] == NULL )
        {
            return;
        }

        for( j = 0; j < p->contours[i]->num_points; j++ )
        {
            p->contours[i]->points[j]->x = p->contours[i]->points[j]->x + x;
            p->contours[i]->points[j]->y = p->contours[i]->points[j]->y + y;
        }
    }

    return;
}

void polygon_erase_contour( struct polygon * p, int ind )
{
    int i = 0;
    int c_i = 0;
    struct contour ** temp_contours = NULL;

    if( p == NULL )
    {
        return;
    }

    if( i >= p->num_contours )
    {
        return;
    }

    free_contour( p->contours[ind] );
    p->contours[ind] = NULL;

    temp_contours = ( struct contour ** ) palloc0(
        sizeof( struct contour * )
      * ( p->num_contours - 1 )
    );

    for( i = 0; i < p->num_contours; i++ )
    {
        if( p->contours[i] != NULL )
        {
            temp_contours[c_i] = p->contours[i];
            c_i++;
        }
    }

    pfree( p->contours );
    p->contours = temp_contours;
    p->num_contours = p->num_contours - 1;
    return;
}

void polygon_add_contour( struct polygon * p, struct contour * c )
{
    if( p == NULL || c == NULL )
    {
        return;
    }

    if( p->contours == NULL )
    {
        if( p->num_contours != 0 )
        {
            return;
        }

        p->contours = ( struct contour ** ) palloc0( sizeof( struct contour * ) );
        p->num_contours = 1;
        p->contours[0] = c;
    }
    else
    {
        p->contours = ( struct contour ** ) repalloc(
            p->contours,
            sizeof( struct contour * )
          * ( p->num_contours + 1 )
        );

        p->contours[p->num_contours] = c;
        p->num_contours = p->num_contours + 1;
    }

    return;
}

struct polygon * new_polygon( void )
{
    struct polygon * p = NULL;

    p = ( struct polygon * ) palloc0( sizeof( struct polygon ) );
    p->num_contours = 0;
    return p;
}

void free_polygon( struct polygon * p )
{
    int i = 0;

    if( p == NULL )
    {
        return;
    }

    if( p->num_contours > 0 )
    {
        for( i = 0; i < p->num_contours; i++ )
        {
            free_contour( p->contours[i] );
        }

        pfree( p->contours );
    }

    pfree( p );
    return;
}

struct segment * sweep_event_get_segment( struct sweep_event * se )
{
    struct segment * s = NULL;

    if( se == NULL )
    {
        return NULL;
    }

    s = new_segment();
    segment_set_begin( s, se->p );
    segment_set_end( s, se->other->p );
    return s;
}

bool sweep_event_below( struct sweep_event * e, Point * p )
{
    double area = 0.0;
    if( e->left )
    {
        area = signed_area_three( e->p, e->other->p, p );

        if( _fp_gt( area, 0.0 ) )
        {
            return true;
        }

        return false;
    }

    area = signed_area_three( e->other->p, e->p, p );

    if( _fp_gt( area, 0.0 ) )
    {
        return true;
    }

    return false;
}

bool sweep_event_above( struct sweep_event * e, Point * p )
{
    return !sweep_event_below( e, p );
}

bool sweep_event_sl_comp_wrapper( void * e1, void * e2 )
{
    return sweep_event_sl_comp( ( struct sweep_event * ) e1, ( struct sweep_event * ) e2 );
}

bool sweep_event_sl_comp( struct sweep_event * e1, struct sweep_event * e2 ) //SweepEventComp
{
    if( _fp_gt( e1->p->x, e2->p->x ) )
    {
        //elog( DEBUG1, "E1: %p, E2: %p 1st cond", e1, e2 );
        return true;
    }

    if( _fp_gt( e2->p->x, e1->p->x ) )
    {
        //elog( DEBUG1, "E1: %p, E2: %p 2nd cond", e1, e2 );
        return false;
    }

    if( !points_equal( e1->p, e2->p ) )
    {
        if( _fp_gt( e1->p->y, e2->p->y ) )
        {
            //elog( DEBUG1, "E1: %p, E2: %p 3rd cond inner", e1, e2 );
            return true;
        }

        //elog( DEBUG1, "E1: %p, E2: %p 3rd cond outer", e1, e2 );
        return false;
    }

    if( e1->left != e2->left )
    {
        //elog( DEBUG1, "E1: %p, E2: %p 4th cond", e1, e2 );
        return e1->left;
    }

    if( sweep_event_above( e1, e2->other->p ) )
    {
        //elog( DEBUG1, "E1: %p, E2: %p 5th cond", e1, e2 );
        return true;
    }
     
    //elog( DEBUG1, "E1: %p, E2: %p fallthrough", e1, e2 );
    return false;
}

bool sweep_event_ev_comp_wrapper( void * e1, void * e2 )
{
    return sweep_event_ev_comp( ( struct sweep_event * ) e1, ( struct sweep_event * ) e2 );
}

bool sweep_event_ev_comp( struct sweep_event * e1, struct sweep_event * e2 ) // SEComp
{
    // returns true if e1 is to the 'left' of e2
    //
    //  left is considered
    //      - strictly to the left of
    //      - directly below if they lie on the same x coordinates
    if( _fp_lt( e1->p->x, e2->p->x ) )
    {
        return true;
    }
    
    if( _fp_lt( e2->p->x, e1->p->x ) )
    {
        return false;
    }

    if( !points_equal( e1->p, e2->p ) )
    {
        return _fp_lt( e1->p->y, e2->p->y );
    }

    if( e1->left != e2->left )
    {
        return !e1->left;
    }

    return sweep_event_below( e1, e2->other->p );
}

bool sweep_event_sl_segment_comp_wrapper( void * e0, void * e1 )
{
    return sweep_event_sl_segment_comp( ( struct sweep_event * ) e0, ( struct sweep_event * ) e1 );
}

bool sweep_event_sl_segment_comp( struct sweep_event * e0, struct sweep_event * e1 ) //SegmentComp
{
    if( sweep_event_equal( e0, e1 ) )
    {
        return false;
    }

    if(
            !_fp_eq( signed_area_three( e0->p, e0->other->p, e1->p ), 0.0 )
         || !_fp_eq( signed_area_three( e0->p, e0->other->p, e1->other->p ), 0.0 )
      )
    {
        if( points_equal( e0->p, e1->p ) )
        {
            return sweep_event_below( e0, e1->other->p );
        }

        if( sweep_event_sl_comp( e0, e1 ) )
        {
            return sweep_event_above( e1, e0->p );
        }

        return sweep_event_below( e0, e1->p );
    }

    if( points_equal( e0->p, e1->p ) )
    {
        return sweep_event_sl_comp( e0, e1 );
    }

    return sweep_event_sl_comp( e0, e1 );
}

bool sweep_event_ev_segment_comp( struct sweep_event * e0, struct sweep_event * e1 ) //SegmentsComp
{
    if( sweep_event_equal( e0, e1 ) )
    {
        return false;
    }

    if(
            !_fp_eq( signed_area_three( e0->p, e0->other->p, e1->p ), 0.0 )
         || !_fp_eq( signed_area_three( e0->p, e0->other->p, e1->other->p ), 0.0 )
      )
    {
        if( points_equal( e0->p, e1->p ) )
        {
            return sweep_event_below( e0, e1->other->p );
        }

        if( sweep_event_ev_comp( e0, e1 ) )
        {
            return sweep_event_below( e0, e1->p );
        }

        return sweep_event_above( e1, e0->p );
    }

    if( points_equal( e0->p, e1->p ) )
    {
        return sweep_event_ev_comp( e0, e1 );
    }

    return sweep_event_ev_comp( e0, e1 );
}

struct sweep_event * new_sweep_event( void )
{
    struct sweep_event * s;
    s = ( struct sweep_event * ) palloc0( sizeof( struct sweep_event ) );
    s->p = ( Point * ) palloc0( sizeof( Point ) );
    s->left = false;
    s->polygon = 0;
    s->in_out = false;
    s->position = 0;
    return s;
}

void free_sweep_event( struct sweep_event * s )
{
    if( s == NULL )
    {
        return;
    }

    if( s->p != NULL )
    {
        pfree( s->p );
    }

    pfree( s );

    return;
}

struct sweep_event ** _manage_ev_buffer( struct sweep_event ** ev, int ev_index )
{
    // Handles extending sweep event buffer by SE_BUFFER_LENGTH
    // based on index.
    if( ( ( ev_index + 1 ) % SE_BUFFER_LENGTH ) == 0 )
    {
        if( ev == NULL )
        {
            elog( DEBUG1, " === made initial ev palloc, size %d == ", SE_BUFFER_LENGTH );
            ev = ( struct sweep_event ** ) palloc0(
                sizeof( struct sweep_event * )
              * SE_BUFFER_LENGTH
            );
        }
        else
        {
            elog( DEBUG1, " === extended ev, size %d == ", SE_BUFFER_LENGTH * ( (int) ( ev_index + 1 / SE_BUFFER_LENGTH ) + 1 ) );
            ev = ( struct sweep_event ** ) repalloc(
                ev,
                sizeof( struct sweep_event * )
              * SE_BUFFER_LENGTH
              * ( (int) ( ( ev_index + 1 ) / SE_BUFFER_LENGTH ) + 1 )
            );
        }
    }
    else
    {
        elog( DEBUG1, "EV not extended :(" );
    }

    return ev;
}

void _sort_ev_buffer( struct sweep_event ** ev, int start_ind, int end_ind )
{
    struct sweep_event * key = NULL;
    struct sweep_event * temp = NULL;
    int i   = 0;
    int j   = 0;
    int k   = 0;

    if( start_ind < end_ind )
    {
        k     = (int) ( end_ind ) / 2;
        temp  = ev[0];
        ev[0] = ev[k];
        ev[k] = temp;
        key   = ev[start_ind];
        i = start_ind + 1;
        j = end_ind;

        while( i <= j )
        {
            while( i <= end_ind && sweep_event_ev_comp( ev[i], key ) )
            {
                i++;
            }

            while( j >= start_ind && !sweep_event_ev_comp( ev[j], key ) )
            {
                j--;
            }

            if( i < j )
            {
                temp  = ev[i];
                ev[i] = ev[j];
                ev[j] = temp;
            }
        }

        temp  = ev[start_ind];
        ev[start_ind] = ev[j];
        ev[j] = temp;

        _sort_ev_buffer( ev, start_ind, j - 1 );
        _sort_ev_buffer( ev, j + 1, end_ind );
    }

    return;
}

/*
 *  Maintains a unique, sorted set of sweep_events
 *   Each element of the ev_set contains its own position.
 *   unlike the previous quick sort, this uses segment_comp as the comparison function
 */
int _se_set_insert(
    struct sweep_event *** set,
    struct sweep_event * se,
    int * len,
    bool (*sort_function)( struct sweep_event *, struct sweep_event * ) // pointer to sorting function
)
{
    //struct sweep_event ** temp = NULL;
    int i = 0;
    int j = 0;
    int ind_offset = 0;

    if( se == NULL )
    {
        return -1;
    }

    //_dump_sweep_event( se );
    if( set == NULL || (*set) == NULL )
    {
        (*set) = ( struct sweep_event ** ) palloc0( sizeof( struct sweep_event * ) );
        (*set)[0] = se;
        se->position = 0;
        //elog( DEBUG1, "Dumping new SET" );
        //_dump_se_set( set, 1 );
        (*len) = 1;
        /*
        elog(
            DEBUG1,
            "Adding SE %p p(%f,%f) op(%f,%f) left: %s to SET: %p",
            se,
            se->p->x,
            se->p->y,
            se->other->p->x,
            se->other->p->y,
            se->left?"true":"false",
            (*set)
        );
        */
        return 1;
    }
    //elog( DEBUG1, "Dumping SET" );
    //_dump_se_set( set, (*len) );
    for( i = 0; i < (*len); i++ )
    {
        // scan for insertion point
        if( !(*sort_function)( (*set)[i], se ) )
        {
            // First time we hit this, (*set)[last_ind] < se <= (*set)[i]
            if( sweep_event_equal( (*set)[i], se ) )
            { // probably need to find a better equality function
                return -1;
            }
            else
            {
                (*set) = ( struct sweep_event ** ) repalloc(
                    (*set),
                    sizeof( struct sweep_event * ) * ( (*len) + 1 )
                );

                ind_offset = 1;
                for( j = (*len); j >= i; j-- )
                {
                    if( j == i )
                    {
                        (*set)[j] = se;
                        ind_offset = 0;
                    }
                    else
                    {
                        (*set)[j] = (*set)[j-ind_offset];
                    }

                    (*set)[j]->position = j;
                }

                (*len)++;
/*
                elog(
                    DEBUG1,
                    "Adding SE %p p(%f,%f) op(%f,%f) left: %s to SET: %p",
                    se,
                    se->p->x,
                    se->p->y,
                    se->other->p->x,
                    se->other->p->y,
                    se->left?"true":"false",
                    (*set)
                );
*/
                return i;
            }
        }

        (*set)[i]->position = i;
    }
    
    (*set) = ( struct sweep_event ** ) repalloc(
        (*set),
        sizeof( struct sweep_event * ) * ( (*len) + 1 )
    );
    (*set)[*len] = se;
    se->position = (*len);
    (*len)++;
/*
    elog(
        DEBUG1,
        "Adding SE %p p(%f,%f) op(%f,%f) left: %s to SET: %p",
        se,
        se->p->x,
        se->p->y,
        se->other->p->x,
        se->other->p->y,
        se->left?"true":"false",
        (*set)
    );
*/
    return 1;
}

// constrict ev buffer to exact fit, clear processed bit and set position index for usage as set
struct sweep_event ** _process_ev_buffer( struct sweep_event ** ev, int ev_index )
{
    struct sweep_event ** new_ev = NULL;
    int i = 0;

    new_ev = ( struct sweep_event ** ) palloc0( sizeof( struct sweep_event * ) * ev_index );
    for( i = 0; i < ev_index; i++ )
    {
        new_ev[i] = ev[i];
        new_ev[i]->position = i;
    }

    pfree( ev );
    return new_ev;
}

void _se_set_remove( struct sweep_event *** set, int position, int * ev_index )
{
    struct sweep_event ** ev_set_temp = NULL;
    int i = 0;
    int ind_offset = 0;

    if( (*set) == NULL || position >= (*ev_index) )
    {
        return;
    }

    ev_set_temp = ( struct sweep_event ** ) palloc0( sizeof( struct sweep_event * ) * ( (*ev_index) - 1 ) );

    for( i = 0; i < (*ev_index) - 1; i++ )
    {
        if( i == position )
        {
            ind_offset++;
        }

        ev_set_temp[i] = (*set)[i + ind_offset];
        ev_set_temp[i]->position = i;
    }

    pfree( (*set) );
    (*set) = ev_set_temp;
    (*ev_index)--;
    return;
}

void polygon_compute_holes( struct polygon * p )
{
    struct sweep_event **  ev            = NULL;
    struct sweep_event **  ev_set        = NULL;
    struct sweep_event *   se_last       = NULL;
    struct sweep_event *   se            = NULL;
    struct segment *       seg           = NULL;
    int                    i             = 0;
    int                    j             = 0;
    int                    ev_index      = 0;
//    int                    ev_set_index  = 0;
    int                    num_processed = 0;
    bool *                 processed     = NULL;
    int *                  hole_of       = NULL;
    int                    insertion_ind = 0;

    if( p == NULL )
    {
        return;
    }

    if( p->num_contours < 2 )
    {
        if( p->num_contours == 1 && contour_clockwise( p->contours[0] ) )
        {
            contour_change_orientation( p->contours[0] );
        }

        return;
    }

    for( i = 0; i < p->num_contours; i++ )
    {
        contour_set_counterclockwise( p->contours[i] );

        for( j = 0; j < p->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( p->contours[i], j );

            // Do not process vertical segments
            if( _fp_eq( seg->p1->x, seg->p2->x ) )
            {
                continue;
            }

            se          = new_sweep_event();
            se->p       = seg->p1;
            se->left    = true;
            se->polygon = i;

            ev = _manage_ev_buffer( ev, ev_index );
            ev[ev_index] = se;
            ev_index++;

            se_last = se;

            se          = new_sweep_event();
            se->p       = seg->p2;
            se->left    = true;
            se->polygon = i;

            se->other      = se_last;
            se_last->other = se;

            ev = _manage_ev_buffer( ev, ev_index );
            ev[ev_index] = se;
            ev_index++;

            if( _fp_lt( se_last->p->x, se->p->x ) )
            {
                se->left        = false;
                se_last->in_out = false;
            }
            else
            {
                se_last->left = false;
                se->in_out    = true;
            }

            se      = NULL;
            se_last = NULL;
        }
    }

    // Sort SEs in place
    _sort_ev_buffer( ev, 0, ev_index - 1 );

    processed = ( bool * ) palloc0( sizeof( bool ) * p->num_contours );
    hole_of   = ( int * ) palloc0( sizeof( int ) * p->num_contours );
    // may need to init ev_set with ev prior to entry

    ev_set = _process_ev_buffer( ev, ev_index );

    for( i = 0; i < ev_index && num_processed < p->num_contours; i++ )
    {
        se = ev[i];

        if( se->left )
        {
            insertion_ind = _se_set_insert( &ev_set, se, &ev_index, &sweep_event_ev_segment_comp );
            if( insertion_ind > 0 )
            {
                ev_index++;
            }

            if( !processed[se->polygon] )
            {
                processed[se->polygon] = true;
                num_processed++;

                if( insertion_ind == 0 )
                {
                    contour_set_counterclockwise( p->contours[se->polygon] );
                }
                else
                {
                    insertion_ind--;
                    se_last = ev_set[insertion_ind];
                    if( !( se_last->in_out) )
                    {
                        hole_of[se->polygon] = se_last->polygon;
                        p->contours[se->polygon]->_external = false;
                        contour_add_hole(
                            p->contours[se_last->polygon],
                            se->polygon
                        );

                        if( contour_counterclockwise( p->contours[se_last->polygon] ) )
                        {
                            contour_set_clockwise( p->contours[se->polygon] );
                        }
                        else
                        {
                            contour_set_counterclockwise( p->contours[se->polygon] );
                        }
                    }
                    else if( hole_of[se->polygon] == hole_of[se_last->polygon] )
                    {
                        hole_of[se->polygon] = hole_of[se_last->polygon];
                        p->contours[se->polygon]->_external = false;
                        contour_add_hole( p->contours[hole_of[se->polygon]], se->polygon );

                        if( contour_counterclockwise( p->contours[hole_of[se->polygon]] ) )
                        {
                            contour_set_clockwise( p->contours[se->polygon] );
                        }
                        else
                        {
                            contour_set_counterclockwise( p->contours[se->polygon] );
                        }
                    }
                    else
                    {
                        contour_set_counterclockwise( p->contours[se->polygon] );
                    }
                }
            }
        }
        else
        {
            _se_set_remove( &ev_set, se->other->position, &ev_index );
        }
    }

    for( i = 0; i < ev_index; i++ )
    {
        free_sweep_event( ev[i] );
        free_sweep_event( ev_set[i] );
    }

    pfree( ev );
    pfree( ev_set );
    pfree( processed );
    pfree( hole_of );

    return;
}

void _dump_polygon( struct polygon * p )
{
    int i = 0;
    struct contour * c = NULL;

    if( p == NULL )
    {
        elog( DEBUG1, "Polygon is NULL" );
        return;
    }

    elog( DEBUG1, "Polygon dump (%p):\n num_contours: %d, contours %p", p, p->num_contours, p->contours );
    for( i = 0; i < p->num_contours; i++ )
    {
        c = p->contours[i];
        _dump_contour( c );
    }

    return;
}

void _dump_sweep_event_dlpq_wrapper( void * event )
{
    _dump_sweep_event( ( struct sweep_event * ) event );
    return;
}

void _dump_sweep_event( struct sweep_event * e )
{
    elog( DEBUG1, "Dumping sweep_event %p", e );
    if( e == NULL )
    {
        return;
    }

    elog( DEBUG1, "L %s p (%f, %f) o ( %f, %f )", e->left ? "T" : "F", e->p->x, e->p->y, e->other->p->x, e->other->p->y );
    /*
    elog( DEBUG1, "left: %s", e->left? "t":"f" );
    elog( DEBUG1, "inside: %s", e->inside?"t":"f" );
    elog( DEBUG1, "polygon %d", e->polygon );
    elog( DEBUG1, "other: %p", e->other );
    elog( DEBUG1, "in_out: %s", e->in_out? "t":"f" );
    elog( DEBUG1, "position: %d", e->position );
    elog( DEBUG1, "edge_type: %d", e->edge_type );
    elog( DEBUG1, "polygon_type: %d", e->polygon_type );
    elog( DEBUG1, "p: (%f,%f)", e->p->x, e->p->y );
    */
    return;
}

void _dump_se_set( struct sweep_event *** ev_set, int ev_index )
{
    struct sweep_event * e = NULL;
    int i = 0;

    elog( DEBUG1, "Dumping SE Set: %p Len: %d", (*ev_set), ev_index );

    for( i = 0; i < ev_index; i ++ )
    {
        e = (*ev_set)[i];
        if( e == NULL )
        {
            elog( DEBUG1, "SES[%d]: NULL", i );
        }
        else
        {
            elog( DEBUG1, "SES[%d]: %p (%f,%f) (%f,%f) %s", i, e, e->p->x, e->p->y, e->other->p->x, e->other->p->y, e->left?"L":"R" );
            //_dump_sweep_event( e );
        }
    }

    return;
}

bool sweep_event_equal( struct sweep_event * e0, struct sweep_event * e1 )
{
    if( e0 == NULL || e1 == NULL )
    {
        if( e0 == NULL && e1 == NULL )
        {
            return true;
        }

        return false;
    }

    if( e0 == e1 )
    {
        return true;
    }

    if( !points_equal( e0->p, e1->p ) )
    {
        return false;
    }

    if( !points_equal( e0->other->p, e1->other->p ) )
    {
        return false;
    }

    if( e0->left != e1->left || e0->inside != e1->inside )
    {
        return false;
    }

    if( e0->polygon != e1->polygon )
    {
        return false;
    }

    if( e0->in_out != e1->in_out )
    {
        return false;
    }

    if( e0->edge_type != e1->edge_type )
    {
        return false;
    }

    return true;
}
