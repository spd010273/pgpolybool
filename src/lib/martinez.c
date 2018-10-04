#include "martinez.h"

void process_segment( struct segment * s, int poly_type, struct pqueue_node ** phead, struct sweep_event *** ev_set, int * ev_index )
{
    struct sweep_event * e1 = NULL;
    struct sweep_event * e2 = NULL;

    if( s == NULL )
    {
        return;
    }

    if( points_equal( s->p1, s->p2 ) )
    {
        return;
    }

    //elog( DEBUG1, "Processing segment" );
    e1 = new_sweep_event();
    e2 = new_sweep_event();

    e1->p            = s->p1;
    e1->left         = true;
    e1->edge_type    = EDGE_TYPE_NORMAL;
    e1->polygon_type = poly_type;
    e1->other        = e2;

    e2->p            = s->p2;
    e2->left         = true;
    e2->edge_type    = EDGE_TYPE_NORMAL;
    e2->polygon_type = poly_type;
    e2->other        = e1;

    //elog( DEBUG1, "New sweep events setup" );
    if( _fp_lt( e1->p->x, e2->p->x ) )
    {
        e2->left = false;
    }
    else if( _fp_gt( e1->p->x, e2->p->x ) )
    {
        e1->left = false;
    }
    else if( _fp_lt( e1->p->y, e2->p->y ) )
    {
        e2->left = false;
    }
    else
    {
        e1->left = false;
    }

    pqueue_push( phead, e1 );
    pqueue_push( phead, e2 );

    _se_set_insert( ev_set, e1, ev_index, &sweep_event_ev_segment_comp );
    _se_set_insert( ev_set, e2, ev_index, &sweep_event_ev_segment_comp );

    pfree( s );
    return;
}

void divide_segment(
    struct sweep_event * e,
    Point * p,
    struct pqueue_node ** phead,
    struct sweep_event *** ev_set,
    int * ev_index
)
{
    struct sweep_event * e0 = NULL; // right
    struct sweep_event * e1 = NULL; // left
    elog( DEBUG1, "DIVIDING SEGMENT" );
    e0 = new_sweep_event();
    e1 = new_sweep_event();
    e0->p = p;
    e0->left = false;
    e0->polygon_type = e->polygon_type;
    e0->other = e;
    e0->edge_type = e->edge_type;

    e1->p = p;
    e1->left = true;
    e1->polygon_type = e->polygon_type;
    e1->other = e->other;
    e1->edge_type = e->other->edge_type;

    if( sweep_event_ev_comp( e1, e->other ) )
    {
        e->other->left = true;
        e1->left = false;
    }
    
    e->other->other = e1;
    e->other = e0;
    _se_set_insert( ev_set, e1, ev_index, &sweep_event_ev_segment_comp );
    _se_set_insert( ev_set, e0, ev_index, &sweep_event_ev_segment_comp );
    pqueue_push( phead, e1 );
    pqueue_push( phead, e0 );

    return;
}

void possible_intersection(
    struct sweep_event * e0,
    struct sweep_event * e1,
    int * num_int,
    struct pqueue_node ** phead,
    struct sweep_event *** ev_set,
    int * ev_length
)
{
    Point * isect_p0 = NULL;
    Point * isect_p1 = NULL;
    struct segment * seg0 = NULL;
    struct segment * seg1 = NULL;
    int num_intersections = 0;
    struct sweep_event ** ev = NULL;
    int ev_index = 0;

    elog( DEBUG1, "POSSIBLE INTERSECTION" );
    //elog( DEBUG1, "Getting segments fron sweep_event e0: %p e1: %p", e0, e1 );
    seg0 = sweep_event_get_segment( e0 );
    seg1 = sweep_event_get_segment( e1 );

    isect_p0 = ( Point * ) palloc0( sizeof( Point ) );
    isect_p1 = ( Point * ) palloc0( sizeof( Point ) );
    //elog( DEBUG1, "Looking for explicit intersection between segments" );
    num_intersections = find_intersection( seg0, seg1, isect_p0, isect_p1 );

    if( num_intersections != 0 )
    {
        return;
    }

    if(
            num_intersections == 1
         && ( points_equal( e0->p, e1->p ) || points_equal( e0->other->p, e1->other->p ) )
      )
    {
        return; // same line segment
    }

    if( num_intersections == 2 && e0->polygon_type == e1->polygon_type )
    {
        return; // segments overlap but are the same poly
    }

    (*num_int) += num_intersections;

    if( num_intersections == 1 )
    {
        if(
               !points_equal( e0->p, isect_p0 )
            && !points_equal( e0->other->p, isect_p0 )
          )
        {
            divide_segment( e0, isect_p0, phead, ev_set, ev_length );
        }

        if(
               !points_equal( e1->p, isect_p0 )
            && !points_equal( e1->other->p, isect_p0 )
          )
        {
            divide_segment( e1, isect_p0, phead, ev_set, ev_length );
        }

        return;
    }

    //elog( DEBUG1, "Pallocing EV Buffer" );
    ev = _manage_ev_buffer( ev, -1 ); // allocate 10 slots, we'll only use at mode 4

    if( points_equal( e0->p, e1->p ) )
    {
        ev[ev_index] = NULL;
        ev_index++;
    }
    else if( sweep_event_sl_comp( e0, e1 ) )
    {
        ev_index += 2;
        ev[ev_index - 2] = e1;
        ev[ev_index - 1] = e0;
    }
    else
    {
        ev_index += 2;
        ev[ev_index - 2] = e0;
        ev[ev_index - 1] = e1;
    }

    if( points_equal( e0->other->p, e1->other->p ) )
    {
        ev_index++;
        ev[ev_index - 1] = NULL;
    }
    else if( sweep_event_sl_comp( e0->other, e1->other ) )
    {
        ev_index += 2;
        ev[ev_index - 2] = e1->other;
        ev[ev_index - 1] = e0->other;
    }
    else
    {
        ev_index += 2;
        ev[ev_index - 2] = e0->other;
        ev[ev_index - 1] = e1->other;
    }

    //elog( DEBUG1, "Post scan logic, ev_index: %d", ev_index );
    if( ev_index == 2 )
    {
        e0->edge_type = EDGE_TYPE_NON_CONTRIBUTING;
        e0->other->edge_type = EDGE_TYPE_NON_CONTRIBUTING;

        if( e0->in_out == e1->in_out )
        {
            e1->edge_type = EDGE_TYPE_SAME_TRANSITION;
            e1->other->edge_type = EDGE_TYPE_SAME_TRANSITION;
        }
        else
        {
            e1->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
            e1->other->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
        }

        pfree( ev );
        return;
    }

    if( ev_index == 3 )
    {
        ev[1]->edge_type = EDGE_TYPE_NON_CONTRIBUTING;
        ev[1]->other->edge_type = EDGE_TYPE_NON_CONTRIBUTING;

        if( ev[0] != NULL )
        {
            if( e0->in_out == e1->in_out )
            {
                ev[0]->other->edge_type = EDGE_TYPE_SAME_TRANSITION;
            }
            else
            {
                ev[0]->other->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
            }
        }
        else
        {
            if( e0->in_out == e1->in_out )
            {
                ev[2]->other->edge_type = EDGE_TYPE_SAME_TRANSITION;
            }
            else
            {
                ev[2]->other->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
            }
        }

        if( ev[0] != NULL )
        {
            divide_segment( ev[0], ev[1]->p, phead, ev_set, ev_length );
        }
        else
        {
            divide_segment( ev[2]->other, ev[1]->p, phead, ev_set, ev_length );
        }

        pfree( ev );
        return;
    }

    if( ev[0] != ev[3]->other )
    {
        ev[1]->edge_type = EDGE_TYPE_NON_CONTRIBUTING;

        if( e0->in_out == e1->in_out )
        {
            ev[2]->edge_type = EDGE_TYPE_SAME_TRANSITION;
        }
        else
        {
            ev[2]->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
        }

        divide_segment( ev[0], ev[1]->p, phead, ev_set, ev_length );
        divide_segment( ev[1], ev[2]->p, phead, ev_set, ev_length );
        pfree( ev );
        return;
    }

    ev[1]->edge_type = EDGE_TYPE_NON_CONTRIBUTING;
    ev[1]->other->edge_type = EDGE_TYPE_NON_CONTRIBUTING;

    divide_segment( ev[0], ev[1]->p, phead, ev_set, ev_length );

    if( e0->in_out == e1->in_out )
    {
        ev[3]->edge_type = EDGE_TYPE_SAME_TRANSITION;
    }
    else
    {
        ev[3]->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
    }

    divide_segment( ev[3]->other, ev[2]->p, phead, ev_set, ev_length );
    pfree( ev );
    return;
}

void compute(
    struct polygon * subject,
    struct polygon * clipping,
    int op,
    struct polygon * result
)
{
    int                        i              = 0;
    int                        j              = 0;
    int                        event_position = 0;
    int                        previous_event = 0;
    int                        next_event     = 0;
    int                        ev_length      = 0;
    int                        colinear_event = 0;
    Point *                    min_subj       = NULL;
    Point *                    max_subj       = NULL;
    Point *                    min_clip       = NULL;
    Point *                    max_clip       = NULL;
    struct pqueue_node **      phead          = NULL;
    struct pqueue_node *       dummy          = NULL;
    struct polygon_connector * pc             = NULL;
    struct sweep_event *       event          = NULL;
    struct segment *           seg            = NULL;
    struct sweep_event **      ev_set         = NULL;
    struct sweep_event **      sl_set         = NULL;
    int                        sl_index       = 0;
    int                        ev_index       = 0;
    int                        num_int        = 0;
    double                     min_max_x      = 0.0;

    if( subject == NULL || clipping == NULL )
    {
        return;
    }

    phead = &dummy;

    if( subject->num_contours * clipping->num_contours == 0 )
    {
        if( op == OP_DIFFERENCE )
        {
            result = subject;
        }

        if( op == OP_UNION )
        {
            result = ( subject->num_contours ) ? clipping : subject;
        }

        return;
    }

    min_subj = ( Point * ) palloc0( sizeof( Point ) );
    max_subj = ( Point * ) palloc0( sizeof( Point ) );
    min_clip = ( Point * ) palloc0( sizeof( Point ) );
    max_clip = ( Point * ) palloc0( sizeof( Point ) );
    polygon_boundingbox( subject, min_subj, max_subj );
    polygon_boundingbox( clipping, min_clip, max_clip );

    if(
            _fp_gt( min_subj->x, max_clip->x )
         || _fp_gt( min_clip->x, max_subj->x )
         || _fp_gt( min_subj->y, max_clip->y )
         || _fp_gt( min_clip->y, max_subj->y )
      )
    {
        // bounding boxes do not overlap
        if( op == OP_DIFFERENCE )
        {
            result = subject;
        }

        if( op == OP_UNION )
        {
            result = subject;

            for( i = 0; i < clipping->num_contours; i++ )
            {
                polygon_add_contour( result, clipping->contours[i] );
            }
        }

        pfree( min_subj );
        pfree( max_subj );
        pfree( min_clip );
        pfree( max_clip );
        return;
    }

    // Generate priority queue
    _dump_polygon( subject );
    _dump_polygon( clipping );
    for( i = 0; i < subject->num_contours; i++ )
    {
        for( j = 0; j < subject->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( subject->contours[i], j );
            process_segment( seg, POLY_TYPE_SUBJECT, phead, &ev_set, &ev_length );
        }
    }

    for( i = 0; i < clipping->num_contours; i++ )
    {
        for( j = 0; j < clipping->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( clipping->contours[i], j );
            process_segment( seg, POLY_TYPE_CLIPPING, phead, &ev_set, &ev_length );
        }
    }

    if( _fp_gt( max_subj->x, max_clip->x ) )
    {
        min_max_x = max_clip->x;
    }
    else
    {
        min_max_x = max_subj->x;
    }

    //_dump_pqueue( phead );
   
    // test pop logic
    struct sweep_event  ** test_set = NULL;
    int ts_ind = 0;
    _dump_se_set( &ev_set, ev_length );
    while( !pqueue_empty( phead ) )
    {
        event = pqueue_pop( phead );
        // insert for sl_sort
        _se_set_insert( &test_set, event, &ts_ind, &sweep_event_sl_comp );
        elog( DEBUG1, "P %p (%f,%f) op (%f,%f)", event, event->p->x, event->p->y, event->other->p->x, event->other->p->y );
    }
 
    _dump_se_set( &test_set, ts_ind );
    // test sl sort

    elog( DEBUG1, " =========== Entering Main Loop ===========\nmin_max_x: %f", min_max_x );    
    pc = new_polygon_connector( NULL, NULL );

    while( !pqueue_empty( phead ) )
    {
        elog( DEBUG1, "================================ LOOP");
        //_dump_pqueue( phead );
        event = pqueue_pop( phead );
        //_dump_sweep_event( event );
        //elog( DEBUG1, "Got event %p, checking basic cases", event );

        if(
                ( op == OP_INTERSECTION && _fp_gt( event->p->x, min_max_x ) )
             || ( op == OP_DIFFERENCE && _fp_gt( event->p->x, max_subj->x ) )
          )
        {
            //elog( DEBUG1, "Early exit for OP_INTERSECTION / OP_DIFFERENCE case" );
            result = polygon_connector_to_polygon( pc );
            free_polygon_connector( pc );
            return;
        }

        //elog( DEBUG1, "Checking union case" );
        if( op == OP_UNION && _fp_gt( event->p->x, min_max_x ) )
        {
            elog( DEBUG1, "Early exit for union case e p(%f,%f)", event->p->x, event->p->y );
            _dump_polygon_connector( pc );
            _dump_pqueue( phead );
            if( !event->left )
            {
                seg = sweep_event_get_segment( event );
                polygon_connector_add_segment( pc, seg );
            }

            while( !pqueue_empty( phead ) )
            {
                event = pqueue_pop( phead );
                //elog( DEBUG1, "Got event %p from pqueue_pop of %p", event, (*phead) );
                if( !event->left )
                {
                    seg = sweep_event_get_segment( event );
                    polygon_connector_add_segment( pc, seg );
                }
            }

            _dump_polygon_connector( pc );
            result = polygon_connector_to_polygon( pc );
            free_polygon_connector( pc );
            return;
        }

        //elog( DEBUG1, "Checking handedness of event" );
        if( event->left )
        {
            //elog( DEBUG1, "Adding event to SE set" );
            //_se_set_insert( &ev_set, event, &ev_length );
            _se_set_insert( &sl_set, event, &sl_index, &sweep_event_sl_segment_comp );
            //_dump_se_set( &ev_set, ev_length );
            event_position = sl_set[0]->position;

            next_event = event_position;
            previous_event = event_position;

            if( !sweep_event_equal( sl_set[previous_event], sl_set[0] ) )
            {
                --previous_event;
                if( previous_event < 0 )
                {
                    previous_event = sl_index - 1;
                }
            }
            else
            {
                previous_event = sl_index - 1; //ev_length
            }

            //elog( DEBUG1, "event in/out & inside logic" ); crashes here at se equal (sl_set[previous_event]->p is undefined
            if(
                    previous_event >= 0
                 && sweep_event_equal( sl_set[previous_event], sl_set[sl_index - 1] ) ) //ev_length )
            {
                event->inside = false;
                event->in_out = false;
            }
            else if( sl_set[previous_event]->edge_type != EDGE_TYPE_NORMAL )
            {
                if( sweep_event_equal( sl_set[previous_event], sl_set[0] ) )
                {
                    event->inside = true;
                    event->in_out = false;
                }
                else
                {
                    colinear_event = previous_event;
                    colinear_event--;
                    if( colinear_event < 0 )
                    {
                        colinear_event = sl_index - 1;
                    }

                    if( sl_set[previous_event]->polygon_type == event->polygon_type )
                    {
                        event->in_out = !(sl_set[previous_event]->in_out);
                        event->inside = !(sl_set[colinear_event]->in_out);
                    }
                    else
                    {
                        event->in_out = !(sl_set[colinear_event]->in_out);
                        event->inside = !(sl_set[previous_event]->in_out);
                    }
                }
            }
            else if( event->polygon_type == sl_set[previous_event]->polygon_type )
            {
                event->inside = sl_set[previous_event]->inside;
                event->in_out = !(sl_set[previous_event]->in_out);
            }
            else
            {
                event->inside = !(sl_set[previous_event]->in_out);
                event->in_out = sl_set[previous_event]->inside;
            }

            //elog( DEBUG1, "Checking possible intersections" );
            ++next_event;

            if( next_event > (sl_index - 1 ) )
            {
                next_event = 0;
            }

            if( !sweep_event_equal( sl_set[next_event], sl_set[sl_index - 1] ) )
            {
                //elog( DEBUG1, "Calling first pi" );
                possible_intersection(
                    event,
                    sl_set[next_event],
                    &num_int,
                    phead,
                    &ev_set,
                    &ev_length
                );
            }

            if( !sweep_event_equal( sl_set[previous_event], sl_set[sl_index - 1] ) )
            {
                possible_intersection(
                    sl_set[previous_event],
                    event,
                    &num_int,
                    phead,
                    &ev_set,
                    &ev_length
                );
            }
            //elog( DEBUG1, "Post possible intersection" );
        }
        else
        {
            //elog( DEBUG1, "colinear & edge logic" );
            colinear_event = event->other->position;
            previous_event = event->other->position;
            next_event = event->other->position;
            ++next_event;

            if( previous_event != 0 )
            {
                --previous_event;
            }
            else
            {
                previous_event = ev_length;
            }

            switch( event->edge_type )
            {
                case EDGE_TYPE_NORMAL:
                    switch( op )
                    {
                        case OP_INTERSECTION:
                            if( event->other->inside )
                            {
                                seg = sweep_event_get_segment( event );
                                polygon_connector_add_segment( pc, seg );
                            }
                            break;
                        case OP_UNION:
                            if( !event->other->inside )
                            {
                                seg = sweep_event_get_segment( event );
                                polygon_connector_add_segment( pc, seg );
                            }
                            break;
                        case OP_DIFFERENCE:
                            if(
                                   ( event->polygon_type == POLY_TYPE_SUBJECT && !event->other->inside )
                                || ( event->polygon_type == POLY_TYPE_CLIPPING && event->other->inside )
                              )
                            {
                                seg = sweep_event_get_segment( event );
                                polygon_connector_add_segment( pc, seg );
                            }
                            break;
                        case OP_XOR:
                            seg = sweep_event_get_segment( event );
                            polygon_connector_add_segment( pc, seg );
                            break;
                    }
                    break;
                case EDGE_TYPE_SAME_TRANSITION:
                    if( op == OP_INTERSECTION || op == OP_UNION )
                    {
                        seg = sweep_event_get_segment( event );
                        polygon_connector_add_segment( pc, seg );
                    }
                    break;
                case EDGE_TYPE_DIFFERENT_TRANSITION:
                    if( op == OP_DIFFERENCE )
                    {
                        seg = sweep_event_get_segment( event );
                        polygon_connector_add_segment( pc, seg );
                    }
                    break;
            }

            _se_set_remove( &sl_set, colinear_event, &sl_index );

            if( next_event < sl_index && previous_event < sl_index )
            {
                possible_intersection(
                    sl_set[previous_event],
                    sl_set[next_event],
                    &num_int,
                    phead,
                    &ev_set,
                    &ev_length
                );
            }
        }
    }

    elog( DEBUG1, "Ended main loop. Pqueue:" );
    //_dump_pqueue( phead );
    _dump_polygon_connector( pc );
    result = polygon_connector_to_polygon( pc );

    for( i = 0; i < pc->open_length; i++ )
    {
        free_connector( pc->open[i] );
    }

    for( i = 0; i < pc->closed_length; i++ )
    {
        free_connector( pc->closed[i] );
    }

    free_polygon_connector( pc );
    return;
}

struct polygon * poly_to_mpoly( POLYGON * p )
{
    struct polygon * mpoly = NULL;
    Point * point = NULL;
    struct contour * c = NULL;
    int i = 0;

    if( p == NULL )
    {
        return NULL;
    }

    mpoly = new_polygon();
    c = new_contour();
    for( i = 0; i < p->npts; i++ )
    {
        point = ( Point * ) palloc0( sizeof( Point ) );
        point->x = p->p[i].x;
        point->y = p->p[i].y;
        contour_add_point( c, point );
    }
    
    polygon_add_contour( mpoly, c );
    free_pgpoly( p );

    return mpoly;
}

void free_pgpoly( POLYGON * p )
{
    if( p == NULL )
    {
        return;
    }

    pfree( p );
    return;
}

POLYGON * mpoly_to_poly( struct polygon * mpoly )
{
    int i = 0;
    int c = 0;
    POLYGON * p = NULL;
    POLYGON ** arr = NULL;
    double area = 0.0;
    double max_area = 0.0;
    int max_area_ind = 0;

    if( mpoly == NULL )
    {
        return NULL;
    }

    if( mpoly->num_contours > 1 )
    {
        arr = palloc0( sizeof( POLYGON * ) * mpoly->num_contours );
    }

    for( c = 0; c < mpoly->num_contours; c++ )
    {
        p = ( POLYGON * ) palloc0(
            offsetof( POLYGON, p )
          + ( sizeof( Point ) * ( mpoly->contours[c]->num_points ) )
        );

        for( i = 0; i < mpoly->contours[c]->num_points; i++ )
        {
            if( mpoly->num_contours == 1 )
            {
                p->p[i].x = mpoly->contours[c]->points[i]->x;
                p->p[i].y = mpoly->contours[c]->points[i]->y;
            }
        }

        if( mpoly->num_contours == 1 )
        {
            free_polygon( mpoly );
            return p;
        }

        arr[c] = p;
        area = contour_area( mpoly->contours[c] );
        if( _fp_gt( area, max_area ) )
        {
            max_area = area;
            max_area_ind = i;
        }
    }


    p = arr[max_area_ind];

    for( i = 0; i < mpoly->num_contours; i++ )
    {
        if( i != max_area_ind )
        {
            pfree( arr[i] );
        }
    }

    pfree( arr );
    free_polygon( mpoly );
    return p;
}
