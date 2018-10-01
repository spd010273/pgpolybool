#include "martinez.h"

void process_segment( struct segment * s, int poly_type, struct pqueue_node ** phead, struct sweep_event *** ev_set, int * ev_index )
{
    struct sweep_event * e1 = NULL;
    struct sweep_event * e2 = NULL;

    if(
           fabs( s->p1->x - s->p2->x ) <= DBL_EPSILON
        && fabs( s->p1->y - s->p2->y ) <= DBL_EPSILON
      )
    {
        return;
    }

    elog( DEBUG1, "Processing segment" );
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

    elog( DEBUG1, "New sweep events setup" );
    if( e1->p->x < e2->p->x )
    {
        e2->left = false;
    }
    else if( e1->p->x > e2->p->x )
    {
        e1->left = false;
    }
    else if( e1->p->y <  e2->p->y )
    {
        e2->left = false;
    }
    else
    {
        e1->left = false;
    }

    elog( DEBUG1, "segments setup, adding sweep event(s) to pqueue" );
    if( phead == NULL || pqueue_empty( phead ) )
    {
        elog( DEBUG1, "setting up new pqueue" );

        (*phead) = new_pqueue( e1 );
    }
    else
    {
        elog( DEBUG1, "Pushing 1st event to pqueue %p", (*phead) );
        pqueue_push( phead, e1 );
    }

    elog( DEBUG1, "Pushing 2nd event to pqueue %p", (*phead) );
    pqueue_push( phead, e2 );

    _se_set_insert( ev_set, e1, ev_index );
    _se_set_insert( ev_set, e2, ev_index );
    return;
}

void divide_segment( struct sweep_event * e, Point * p, struct pqueue_node ** phead )
{
    struct sweep_event * e0 = NULL;
    struct sweep_event * e1 = NULL;

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

    if( sweep_event_comp( e1, e->other ) )
    {
        e->other->left = true;
        e->left = false;
    }

    e->other->other = e1;
    e->other = e0;
    pqueue_push( phead, e1 );
    pqueue_push( phead, e0 );

    return;
}

void possible_intersection( struct sweep_event * e0, struct sweep_event * e1, int * num_int, struct pqueue_node ** phead )
{
    Point * isect_p0 = NULL;
    Point * isect_p1 = NULL;
    struct segment * seg0 = NULL;
    struct segment * seg1 = NULL;
    int num_intersections = 0;
    struct sweep_event ** ev = NULL;
    int ev_index = 0;

    elog( DEBUG1, "Getting segments fron sweep_event e0: %p e1: %p", e0, e1 );
    seg0 = sweep_event_get_segment( e0 );
    seg1 = sweep_event_get_segment( e1 );

    isect_p0 = ( Point * ) palloc0( sizeof( Point ) );
    isect_p1 = ( Point * ) palloc0( sizeof( Point ) );
    elog( DEBUG1, "Looking for explicit intersection between segments" );
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
            divide_segment( e0, isect_p0, phead );
        }

        if(
               !points_equal( e1->p, isect_p0 )
            && !points_equal( e1->other->p, isect_p0 )
          )
        {
            divide_segment( e1, isect_p0, phead );
        }

        return;
    }

    elog( DEBUG1, "Pallocing EV Buffer" );
    ev = _manage_ev_buffer( ev, -1 ); // allocate 10 slots, we'll only use at mode 4

    if( points_equal( e0->p, e1->p ) )
    {
        ev[ev_index] = NULL;
        ev_index++;
    }
    else if( sweep_event_comp( e0, e1 ) )
    {
        ev_index += 2;
        ev[ev_index - 2] = e1;
        ev[ev_index - 1] = e0;
    }
    else
    {
        ev_index += 2;
        ev[ev_index - 2 ] = e0;
        ev[ev_index - 1 ] = e1;
    }

    if( points_equal( e0->other->p, e1->other->p ) )
    {
        ev_index++;
        ev[ev_index - 1] = NULL;
    }
    else if( sweep_event_comp( e0->other, e1->other ) )
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

    elog( DEBUG1, "Post scan logic, ev_index: %d", ev_index );
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
            divide_segment( ev[0], ev[1]->p, phead );
        }
        else
        {
            divide_segment( ev[2]->other, ev[1]->p, phead );
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

        divide_segment( ev[0], ev[1]->p, phead );
        divide_segment( ev[1], ev[2]->p, phead );
        pfree( ev );
        return;
    }

    ev[1]->edge_type = EDGE_TYPE_NON_CONTRIBUTING;
    ev[1]->other->edge_type = EDGE_TYPE_NON_CONTRIBUTING;

    divide_segment( ev[0], ev[1]->p, phead );

    if( e0->in_out == e1->in_out )
    {
        ev[3]->edge_type = EDGE_TYPE_SAME_TRANSITION;
    }
    else
    {
        ev[3]->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
    }

    divide_segment( ev[3]->other, ev[2]->p, phead );
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
//    struct connector *         conn           = NULL;
    struct sweep_event *       event          = NULL;
    struct segment *           seg            = NULL;
    struct sweep_event **      ev_set         = NULL;
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

    elog( DEBUG1, "Eliminating trivial cases" );
    min_subj = ( Point * ) palloc0( sizeof( Point ) );
    max_subj = ( Point * ) palloc0( sizeof( Point ) );
    min_clip = ( Point * ) palloc0( sizeof( Point ) );
    max_clip = ( Point * ) palloc0( sizeof( Point ) );
    polygon_boundingbox( subject, min_subj, max_subj );
    polygon_boundingbox( clipping, min_clip, max_clip );

    if(
            min_subj->x > max_clip->x
         || min_clip->x > max_subj->x
         || min_subj->y > max_clip->y
         || min_clip->y > max_subj->y
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

    elog( DEBUG1, "============================== PHEAD: %p", phead );
    // Generate priority queue
    elog( DEBUG1, "Subject has %d contours", subject->num_contours );
    elog( DEBUG1, "Generating pqueue (subject)" );
    for( i = 0; i < subject->num_contours; i++ )
    {
        elog( DEBUG1, "Contour %d has %d points", i, subject->contours[i]->num_points );
        for( j = 0; j < subject->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( subject->contours[i], j );
            elog( DEBUG1, "Got segment: (%f,%f),(%f,%f)", seg->p1->x, seg->p1->y, seg->p2->x, seg->p2->y );
            process_segment( seg, POLY_TYPE_SUBJECT, phead, &ev_set, &ev_length );
        }
    }

    elog( DEBUG1, "============================== PHEAD: %p", phead );
    elog( DEBUG1, "Clipping has %d contours", clipping->num_contours );
    elog( DEBUG1, "Generating pqueue (clipping)" );
    for( i = 0; i < clipping->num_contours; i++ )
    {
        elog( DEBUG1, "Contour %d has %d points", i, clipping->contours[i]->num_points );
        for( j = 0; j < clipping->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( clipping->contours[i], j );
            elog( DEBUG1, "Got segment: (%f,%f),(%f,%f)", seg->p1->x, seg->p1->y, seg->p2->x, seg->p2->y );
            process_segment( seg, POLY_TYPE_CLIPPING, phead, &ev_set, &ev_length );
        }
    }

    min_max_x = ( max_subj->x > max_clip->x ) ? max_clip->x : max_subj->x;

    elog( DEBUG1, "============================== PHEAD: %p", phead );
    elog( DEBUG1, "===Entering main loop==" );
    //elog( DEBUG1, "================== PQUEUE: ======================" );
    //_dump_pqueue( phead );
    //elog( DEBUG1, "================== SE SET: ======================" );
    //_dump_se_set( &ev_set, ev_length );
    while( !pqueue_empty( phead ) )
    {
        //elog( DEBUG1, "popping event from priority queue" );
        //elog( DEBUG1, "Phead: ** %p, * %p", phead, (*phead) );
        //_dump_pqueue( phead );
        event = pqueue_pop( phead );
        //elog( DEBUG1, "Phead: ** %p, * %p", phead, (*phead) );
        //_dump_pqueue( phead );
        //_dump_sweep_event( event );
        elog( DEBUG1, "Got event %p, checking basic cases", event );

        if(
                ( op = OP_INTERSECTION && ( event->p->x > min_max_x ) )
             || ( op = OP_DIFFERENCE && event->p->x > max_subj->x )
          )
        {
            result = polygon_connector_to_polygon( pc );
            return;
        }

        elog( DEBUG1, "Checking union case" );
        if( op == OP_UNION && event->p->x > min_max_x )
        {
            elog( DEBUG1, "Early exit for union case" );
            if( !event->left )
            {
                seg = sweep_event_get_segment( event );
                polygon_connector_add_segment( pc, seg );
            }

            while( !pqueue_empty( phead ) )
            {
                event = pqueue_pop( phead );
                if( !event->left )
                {
                    seg = sweep_event_get_segment( event );
                    polygon_connector_add_segment( pc, seg );
                }
            }

            result = polygon_connector_to_polygon( pc );
            return;
        }

        elog( DEBUG1, "Checking handedness of event" );
        if( event->left )
        {
            elog( DEBUG1, "Adding event to SE set" );
            _se_set_insert( &ev_set, event, &ev_length );
            //_dump_se_set( &ev_set, ev_length );
            event_position = ev_set[0]->position;

            next_event = event_position;
            previous_event = event_position;

            if( previous_event != 0 )
            {
                --previous_event;
            }
            else
            {
                previous_event = ev_length;
            }

            elog( DEBUG1, "event in/out & inside logic" );
            if( previous_event == ev_length )
            {
                event->inside = false;
                event->in_out = false;
            }
            else if( ev_set[previous_event]->edge_type == EDGE_TYPE_NORMAL )
            {
                if( previous_event == 0 )
                {
                    event->inside = true;
                    event->in_out = false;
                }
                else
                {
                    colinear_event = previous_event;
                    colinear_event--;

                    if( ev_set[previous_event]->polygon_type == event->polygon_type )
                    {
                        event->in_out = !(ev_set[previous_event]->in_out);
                        event->inside = !(ev_set[colinear_event]->in_out);
                    }
                    else
                    {
                        event->in_out = !(ev_set[colinear_event]->in_out);
                        event->inside = !(ev_set[previous_event]->in_out);
                    }
                }
            }
            else if( event->polygon_type == ev_set[previous_event]->polygon_type )
            {
                event->inside = ev_set[previous_event]->inside;
                event->in_out = !(ev_set[previous_event]->in_out);
            }
            else
            {
                event->inside = !(ev_set[previous_event]->in_out);
                event->in_out = ev_set[previous_event]->inside;
            }

            elog( DEBUG1, "Checking possible intersections" );
            if( ++next_event < ev_length )
            {
                elog( DEBUG1, "Calling first pi" );
                possible_intersection( event, ev_set[next_event], &num_int, phead );
            }

            if( previous_event < ev_length )
            {
                possible_intersection( ev_set[previous_event], event, &num_int, phead );
            }
            elog( DEBUG1, "Post possible intersection" );
        }
        else
        {
            elog( DEBUG1, "colinear & edge logic" );
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

            _se_set_remove( &ev_set, colinear_event, ev_index );
            ev_index--;

            if( next_event < ev_index && previous_event < ev_index )
            {
                possible_intersection( ev_set[previous_event], ev_set[next_event], &num_int, phead );
            }
        }
    }

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
          + sizeof( Point ) * ( mpoly->contours[c]->num_points )
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
        if( area > max_area )
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
