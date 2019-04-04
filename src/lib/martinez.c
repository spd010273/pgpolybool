/*------------------------------------------------------------------------------
 * martinez.c
 *     Martinez-Rueda-Feito algorithm for polygon clipping ( compute() ) and
 *     segment processing functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      martinez.c
 *
 *------------------------------------------------------------------------------
 */

#include "martinez.h"

void process_segment(
    struct segment *       s,
    unsigned int           poly_type,
    struct dlpq *          phead,
    struct sweep_event *** ev_set,
    unsigned int *         ev_index
)
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

    e1 = new_sweep_event();
    e2 = new_sweep_event();

    e1->p            = s->p1;
    e1->left         = true;
    e1->edge_type    = EDGE_TYPE_NORMAL;
    e1->polygon_type = poly_type;
    e1->other        = e2;
    e1->in_out       = true;

    e2->p            = s->p2;
    e2->left         = true;
    e2->edge_type    = EDGE_TYPE_NORMAL;
    e2->polygon_type = poly_type;
    e2->other        = e1;
    e2->in_out       = true;

    if( e1->p->x < e2->p->x )
    {
        e2->left = false;
    }
    else if( e1->p->x > e2->p->x )
    {
        e1->left = false;
    }
    else if( e1->p->y < e2->p->y )
    {
        e2->left = false;
    }
    else
    {
        e1->left = false;
    }

    dlpq_push( phead, ( void * ) e1 );
    dlpq_push( phead, ( void * ) e2 );

    _se_set_insert( ev_set, e1, ev_index, &sweep_event_ev_segment_comp );
    _se_set_insert( ev_set, e2, ev_index, &sweep_event_ev_segment_comp );

    pfree( s );
    return;
}

void divide_segment(
    struct sweep_event *   e,
    Point *                p,
    struct dlpq *          phead,
    struct sweep_event *** ev_set,
    unsigned int *         ev_index
)
{
    struct sweep_event * e0 = NULL; // right
    struct sweep_event * e1 = NULL; // left

#ifdef DEBUG
    elog( DEBUG1, "DIVIDING SEGMENT, E %p, P %p", e, p );
#endif // DEBUG

    e0 = new_sweep_event();
    e1 = new_sweep_event();

    e0->p->x         = p->x;
    e0->p->y         = p->y;
    e0->left         = false;
    e0->polygon_type = e->polygon_type;
    e0->other        = e;
    e0->edge_type    = e->edge_type;
    e0->inside       = true;
    e0->in_out       = true;

    e1->p->x         = p->x;
    e1->p->y         = p->y;
    e1->left         = true;
    e1->polygon_type = e->polygon_type;
    e1->other        = e->other;
    e1->edge_type    = e->other->edge_type;
    e1->inside       = true;
    e1->in_out       = true;

    // was ev comp
    if( sweep_event_sl_comp( e1, e->other ) )
    {
        e->other->left = true;
        e1->left = false;
    }

    e->other->other = e1;
    e->other = e0;
#ifdef DEBUG
    elog( DEBUG1, "Divided right segment" );
    _dump_sweep_event( e0 );
    elog( DEBUG1, "Divided left segment" );
    _dump_sweep_event( e1 );
    //_dlpq_debug( phead );
#endif // DEBUG

    _se_set_insert( ev_set, e1, ev_index, &sweep_event_ev_segment_comp );
    _se_set_insert( ev_set, e0, ev_index, &sweep_event_ev_segment_comp );

    dlpq_push( phead, ( void * ) e1 );
    dlpq_push( phead, ( void * ) e0 );

    return;
}

void possible_intersection(
    struct sweep_event *   e0,
    struct sweep_event *   e1,
    unsigned int *         num_int,
    struct dlpq *          phead,
    struct sweep_event *** ev_set,
    unsigned int *         ev_length
)
{
    struct sweep_event ** ev                = NULL;
    struct segment *      seg0              = NULL;
    struct segment *      seg1              = NULL;
    Point *               isect_p0          = NULL;
    Point *               isect_p1          = NULL;
    unsigned int          num_intersections = 0;
    unsigned int          ev_index          = 0;

    seg0 = sweep_event_get_segment( e0 );
    seg1 = sweep_event_get_segment( e1 );

    isect_p0 = ( Point * ) palloc0( sizeof( Point ) );
    isect_p1 = ( Point * ) palloc0( sizeof( Point ) );

    if( isect_p0 == NULL || isect_p1 == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not allocate intersection points" )
            )
        );
    }

    num_intersections = find_intersection( seg0, seg1, isect_p0, isect_p1 );

#ifdef DEBUG
    elog(
        DEBUG1,
        "Locating intersections between p (%f,%f) o (%f,%f) "\
        "and p (%f,%f) o (%f,%f)",
        e0->p->x,
        e0->p->y,
        e0->other->p->x,
        e0->other->p->y,
        e1->p->x,
        e1->p->y,
        e1->other->p->x,
        e1->other->p->y
    );
    elog(
        DEBUG1,
        "Find intersections %d, %f,%f %f,%f",
        num_intersections,
        isect_p0->x,
        isect_p0->y,
        isect_p1->x,
        isect_p1->y
    );
#endif // DEBUG

    if( num_intersections == 0 )
    {
        return;
    }

    if(
            num_intersections == 1
         && (
                points_equal( e0->p, e1->p )
             || points_equal( e0->other->p, e1->other->p )
            )
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

    // allocate 10 slots, we'll only use at mode 4
    ev = _manage_ev_buffer( ev, 0 );

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

    if( ev_index == 2 )
    {
        e0->edge_type        = EDGE_TYPE_NON_CONTRIBUTING;
        e0->other->edge_type = EDGE_TYPE_NON_CONTRIBUTING;

        if( e0->in_out == e1->in_out )
        {
            e1->edge_type        = EDGE_TYPE_SAME_TRANSITION;
            e1->other->edge_type = EDGE_TYPE_SAME_TRANSITION;
        }
        else
        {
            e1->edge_type        = EDGE_TYPE_DIFFERENT_TRANSITION;
            e1->other->edge_type = EDGE_TYPE_DIFFERENT_TRANSITION;
        }

        pfree( ev );
        return;
    }

    if( ev_index == 3 )
    {
        ev[1]->edge_type        = EDGE_TYPE_NON_CONTRIBUTING;
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

    ev[1]->edge_type        = EDGE_TYPE_NON_CONTRIBUTING;
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

struct polygon * compute(
    struct polygon * subject,
    struct polygon * clipping,
    short int        op
)
{
    unsigned int               i              = 0;
    unsigned int               j              = 0;
    unsigned int               num_int        = 0;
    unsigned int               ev_length      = 0;
    unsigned int               event_position = 0;
    unsigned int               previous_event = 0;
    unsigned int               next_event     = 0;
    unsigned int               colinear_event = 0;
    double                     min_max_x      = 0.0;
    Point *                    min_subj       = NULL;
    Point *                    max_subj       = NULL;
    Point *                    min_clip       = NULL;
    Point *                    max_clip       = NULL;
    struct dlpq *              phead          = NULL;
    struct dlpq *              sl_head        = NULL; // Status line
    struct segment *           seg            = NULL;
    struct polygon_connector * pc             = NULL;
    struct sweep_event *       event          = NULL;
    struct sweep_event **      ev_set         = NULL;
    struct polygon *           result         = NULL;

    if( subject == NULL || clipping == NULL )
    {
        return NULL;
    }

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

        return result;
    }

    phead = new_dlpq( &sweep_event_sl_comp_wrapper );
#ifdef DEBUG
    _dlpq_setup_debug( phead, &_dump_sweep_event_dlpq_wrapper );
#endif
    min_subj = ( Point * ) palloc0( sizeof( Point ) );
    max_subj = ( Point * ) palloc0( sizeof( Point ) );
    min_clip = ( Point * ) palloc0( sizeof( Point ) );
    max_clip = ( Point * ) palloc0( sizeof( Point ) );

    if(
            min_subj == NULL
         || max_subj == NULL
         || min_clip == NULL
         || max_clip == NULL
      )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg(
                    "Could not allocate reference points"\
                    " for clipping optimization"
                )
            )
        );
    }


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
        return result;
    }

    // Generate priority queue
    //_dump_polygon( subject );
    //_dump_polygon( clipping );
    for( i = 0; i < subject->num_contours; i++ )
    {
        for( j = 0; j < subject->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( subject->contours[i], j );
            process_segment(
                seg,
                POLY_TYPE_SUBJECT,
                phead,
                &ev_set,
                &ev_length
            );
        }
    }

    for( i = 0; i < clipping->num_contours; i++ )
    {
        for( j = 0; j < clipping->contours[i]->num_points; j++ )
        {
            seg = contour_get_segment( clipping->contours[i], j );
            process_segment(
                seg,
                POLY_TYPE_CLIPPING,
                phead,
                &ev_set,
                &ev_length
            );
        }
    }

    if( max_subj->x > max_clip->x )
    {
        min_max_x = max_clip->x;
    }
    else
    {
        min_max_x = max_subj->x;
    }

    sl_head = new_dlpq( &sweep_event_sl_segment_comp_wrapper_inverted );

#ifdef DEBUG
    elog(
        DEBUG1,
        " =========== Entering Main Loop ===========\nmin_max_x: %f",
        min_max_x
    );
    _dlpq_debug( phead );
    _dlpq_setup_debug( sl_head, &_dump_sweep_event_dlpq_wrapper );
#endif // DEBUG

    pc = new_polygon_connector( NULL, NULL );

    // TODO: verify all events are freed
    while( !dlpq_empty( phead ) )
    {
        event = ( struct sweep_event * ) dlpq_pop( phead );

#ifdef DEBUG
        elog( DEBUG1, "================================ LOOP");
        elog( DEBUG1, "Got event %p :", event );
        _dump_sweep_event( event );
#endif // DEBUG

        if(
                ( op == OP_INTERSECTION && event->p->x > min_max_x   )
             || ( op == OP_DIFFERENCE   && event->p->x > max_subj->x )
          )
        {
#ifdef DEBUG
            elog(
                DEBUG1,
                "Early exit for OP_INTERSECTION / OP_DIFFERENCE case"
            );
#endif // DEBUG
            result = polygon_connector_to_polygon( pc );
            pfree( min_subj );
            pfree( max_subj );
            pfree( min_clip );
            pfree( max_clip );
            free_polygon_connector( pc );
            free_dlpq( &sl_head );
            free_dlpq( &phead );

            return result;
        }

#ifdef DEBUG
        elog( DEBUG1, "Checking union case" );
#endif // DEBUG

        if( op == OP_UNION && event->p->x > min_max_x )
        {
#ifdef DEBUG
            elog(
                DEBUG1,
                "Early exit for union case e p(%f,%f) o(%f,%f)",
                event->p->x,
                event->p->y,
                event->other->p->x,
                event->other->p->y
            );
            elog( DEBUG1, "status line state:" );
            _dlpq_debug( sl_head );
            elog( DEBUG1, "polygon connector state:" );
            _dump_polygon_connector( pc );
#endif // DEBUG

            if( !event->left )
            {
                seg = sweep_event_get_segment( event );
                polygon_connector_add_segment( pc, seg );
            }

            while( !dlpq_empty( phead ) )
            {
                event = ( struct sweep_event * ) dlpq_pop( phead );
#ifdef DEBUG
                elog(
                    DEBUG1,
                    "Got event %p from dlpq_pop of %p",
                    event,
                    phead
                );
#endif // DEBUG
                if( !event->left )
                {
                    seg = sweep_event_get_segment( event );
                    polygon_connector_add_segment( pc, seg );
                }
            }

            result = polygon_connector_to_polygon( pc );
#ifdef DEBUG
            _dump_polygon_connector( pc );
            _dump_polygon( result );
#endif // DEBUG
            free_polygon_connector( pc );
            free_dlpq( &sl_head );
            free_dlpq( &phead );
            pfree( min_subj );
            pfree( max_subj );
            pfree( min_clip );
            pfree( max_clip );
            return result;
        }
#ifdef DEBUG
        elog( DEBUG1, "Checking handedness of event" );
#endif // DEBUG
        if( event->left )
        {
#ifdef DEBUG
            elog(
                DEBUG1,
                "LS: P %d, N %d, ep %d S: %d",
                previous_event,
                next_event,
                event_position,
                sl_head->size
            );

            elog( DEBUG1, "Adding event to SE set" );
#endif // DEBUG
            dlpq_push( sl_head, event );

            event_position = dlpq_get_position( sl_head, event );
            next_event     = event_position;
            previous_event = event_position;

            if( previous_event != 0 )
            {
                previous_event--;
            }
            else
            {
                previous_event = sl_head->size;
            }

#ifdef DEBUG
            elog( DEBUG1, "event in/out & inside logic" );
            elog( DEBUG1, "status line state:" );
            _dlpq_debug( sl_head );
            elog(
                DEBUG1,
                "P %d, N %d, ep %d S: %d",
                previous_event,
                next_event,
                event_position,
                sl_head->size
            );
#endif //DEBUG

            if( sl_head->size == previous_event )
            {
#ifdef DEBUG
                elog( DEBUG1, "Event is not inside not inout" );
#endif // DEBUG
                event->inside = false;
                event->in_out = false;
            }
            else if(
                     (
                        (struct sweep_event *) dlpq_peek_position(
                            sl_head,
                            previous_event
                        )
                     )->edge_type != EDGE_TYPE_NORMAL
                   )
            {
                if( previous_event == 0 )
                {
#ifdef DEBUG
                    elog( DEBUG1, "Event is inside, not inout" );
#endif // DEBUG
                    event->inside = true;
                    event->in_out = false;
                }
                else
                {
                    colinear_event = previous_event;

                    if( colinear_event == 0 )
                    {
                        colinear_event = 0;
                    }
                    else
                    {
                        colinear_event--;
                    }

                    if(
                        (
                         (struct sweep_event *) dlpq_peek_position(
                             sl_head,
                             previous_event
                         )
                        )->polygon_type == event->polygon_type
                      )
                    {
                        event->in_out = !(
                            (
                             (struct sweep_event *) dlpq_peek_position(
                                 sl_head,
                                 previous_event
                             )
                            )->in_out
                        );
                        event->inside = !(
                            (
                             (struct sweep_event *) dlpq_peek_position(
                                 sl_head,
                                 colinear_event
                             )
                            )->in_out
                        );
#ifdef DEBUG
                        elog( DEBUG1, "Event is in first colinear cond" );
#endif // DEBUG
                    }
                    else
                    {
                        event->in_out = !(
                            (
                             (struct sweep_event *) dlpq_peek_position(
                                 sl_head,
                                 colinear_event
                             )
                            )->in_out
                        );
                        event->inside = !(
                            (
                             (struct sweep_event *) dlpq_peek_position(
                                 sl_head,
                                 previous_event
                             )
                            )->in_out
                        );
#ifdef DEBUG
                        elog( DEBUG1, "Event is in second colinear cond" );
#endif // DEBUG
                    }
                }
            }
            else if(
                        (
                         (struct sweep_event *) dlpq_peek_position(
                             sl_head,
                             previous_event
                         )
                        )->polygon_type == event->polygon_type
                   )
            {
                event->inside = (
                 (struct sweep_event *) dlpq_peek_position(
                     sl_head,
                     previous_event
                 )
                )->inside;
                event->in_out = !(
                 (struct sweep_event *) dlpq_peek_position(
                     sl_head,
                     previous_event
                 )
                )->in_out;
#ifdef DEBUG
                elog( DEBUG1, "Event is in first polytype check cond" );
#endif // DEBUG
            }
            else
            {
                event->inside = !(
                 (struct sweep_event *) dlpq_peek_position(
                     sl_head,
                     previous_event
                 )
                )->in_out;
                event->in_out = (
                 (struct sweep_event *) dlpq_peek_position(
                     sl_head,
                     previous_event
                 )
                )->inside;
#ifdef DEBUG
                elog( DEBUG1, "Event is in second polytype check cond" );
#endif // DEBUG
            }

#ifdef DEBUG
            elog( DEBUG1, "Checking possible intersections" );
#endif // DEBUG

            if( ( next_event + 1 ) >= sl_head->size  )
            {
                next_event = sl_head->size;
            }
            else
            {
                next_event++;
            }

            if( next_event != sl_head->size )
            {
#ifdef DEBUG
                elog( DEBUG1, "Calling first pi" );
#endif // DEBUG
                possible_intersection(
                    event,
                    (struct sweep_event *) dlpq_peek_position(
                        sl_head,
                        next_event
                    ),
                    &num_int,
                    phead,
                    &ev_set,
                    &ev_length
                );
#ifdef DEBUG
                elog(
                    DEBUG1,
                    "============== 1 POST POSSIBLE INTERSECTION ============="
                );
                _dlpq_debug( phead );
                elog(
                    DEBUG1,
                    "========================================================="
                );
#endif // DEBUG
            }

            if( previous_event != sl_head->size )
            {
                possible_intersection(
                    (struct sweep_event *) dlpq_peek_position(
                        sl_head,
                        previous_event
                    ),
                    event,
                    &num_int,
                    phead,
                    &ev_set,
                    &ev_length
                );
#ifdef DEBUG
                elog(
                    DEBUG1,
                    "============== 2 POST POSSIBLE INTERSECTION ============="
                );
                _dlpq_debug( phead );
                elog(
                    DEBUG1,
                    "========================================================="
                );
#endif // DEBUG
            }
        }
        else
        {
#ifdef DEBUG
            elog( DEBUG1, "colinear & edge logic" );
#endif // DEBUG
            colinear_event = dlpq_get_position( sl_head, event->other );
            previous_event = colinear_event;
            next_event     = colinear_event;

            if( next_event >= sl_head->size )
            {
                next_event = sl_head->size;
            }
            else
            {
                next_event++;
            }

            if( previous_event > 0 )
            {
                previous_event = 0;
            }
            else
            {
                previous_event = sl_head->size;
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
                                  (
                                      event->polygon_type == POLY_TYPE_SUBJECT
                                   && !event->other->inside
                                  )
                               || (
                                      event->polygon_type == POLY_TYPE_CLIPPING
                                   && event->other->inside
                                  )
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

            dlpq_remove(
                sl_head,
                dlpq_peek_position(
                    sl_head,
                    colinear_event
                )
            );

            if(
                    next_event < sl_head->size - 1
                 && previous_event < sl_head->size - 1
                 && sl_head->size != 0
              )
            {
                possible_intersection(
                    (struct sweep_event *) dlpq_peek_position(
                        sl_head,
                        previous_event
                    ),
                    (struct sweep_event *) dlpq_peek_position(
                        sl_head,
                        next_event
                    ),
                    &num_int,
                    phead,
                    &ev_set,
                    &ev_length
                );
            }
        }
    }

#ifdef DEBUG
    elog( DEBUG1, "Ended main loop. dlpq:" );
    _dump_polygon_connector( pc );
#endif // DEBUG

    result = polygon_connector_to_polygon( pc );

    free_polygon_connector( pc );
    free_dlpq( &sl_head );
    free_dlpq( &phead );
    pfree( min_subj );
    pfree( max_subj );
    pfree( min_clip );
    pfree( max_clip );

    return result;
}

struct polygon * poly_to_mpoly( POLYGON * p )
{
    struct polygon * mpoly = NULL;
    Point *          point = NULL;
    struct contour * c     = NULL;
    unsigned int     i     = 0;

    if( p == NULL )
    {
        return NULL;
    }

    mpoly = new_polygon();
    c     = new_contour();

    for( i = 0; i < p->npts; i++ )
    {
        point = ( Point * ) palloc0( sizeof( Point ) );

        if( point == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg(
                        "Could not allocate point fror polygon conversion"
                    )
                )
            );
        }

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

POLYGON ** mpoly_to_poly( struct polygon * mpoly, bool single_result )
{
    unsigned int i            = 0;
    unsigned int c            = 0;
    unsigned int max_area_ind = 0;
    POLYGON *    p            = NULL;
    POLYGON **   arr          = NULL;
    POLYGON **   result       = NULL;
    double       area         = 0.0;
    double       max_area     = 0.0;
    
    if( mpoly == NULL )
    {
        return NULL;
    }

    if( single_result )
    {
        result = ( POLYGON ** ) palloc0( sizeof( POLYGON * ) );
    
        if( result == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not create polygon result array" )
                )
            );
        }
    }

    if( mpoly->num_contours > 1 )
    {
        arr = ( POLYGON ** ) palloc0(
            sizeof( POLYGON * ) * mpoly->num_contours
        );

        if( arr == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not allocate array for polygon contours" )
                )
            );
        }
    }

    for( c = 0; c < mpoly->num_contours; c++ )
    {
        p = ( POLYGON * ) palloc0(
            offsetof( POLYGON, p )
          + ( sizeof( Point ) * ( mpoly->contours[c]->num_points ) )
        );

        if( p == NULL )
        {
            ereport(
                ERROR,
                (
                    errcode( ERRCODE_OUT_OF_MEMORY ),
                    errmsg( "Could not allocate new output polygon" )
                )
            );
        }

        for( i = 0; i < mpoly->contours[c]->num_points; i++ )
        {
            p->p[i].x = mpoly->contours[c]->points[i]->x;
            p->p[i].y = mpoly->contours[c]->points[i]->y;
            p->npts   = mpoly->contours[c]->num_points;
        }

        if( mpoly->num_contours == 1 )
        {
            free_polygon( mpoly );
            result[0] = p;
            return result;
        }

        arr[c] = p;
        area   = contour_area( mpoly->contours[c] );

        if( area > max_area )
        {
            max_area     = area;
            max_area_ind = c;
        }
    }

    if( single_result )
    {
        p = arr[max_area_ind];

        for( i = 0; i < mpoly->num_contours; i++ )
        {
            if( i != max_area_ind )
            {
                pfree( arr[i] );
            }
        }

        pfree( arr );

        result[0] = p;
    }
    else
    {
        result = arr;
    }

    free_polygon( mpoly );

    return result;
}
