#ifndef POLYGON_H
#define POLYGON_H

#include "contour.h"
#include "segment.h"
#include "util.h"

#define SE_BUFFER_LENGTH 10

struct polygon {
    struct contour ** contours;
    int num_contours;
};

struct sweep_event {
    Point * p;
    bool left;
    bool inside;
    unsigned int polygon;
    struct sweep_event * other;
    bool in_out;
    unsigned int position;
    short int edge_type;
    short int polygon_type;
};

// Polygon functions
extern unsigned int polygon_num_points( struct polygon * );
extern void polygon_boundingbox( struct polygon *, Point *, Point * );
extern void polygon_move( struct polygon *, double, double );
extern void polygon_erase_contour( struct polygon *, unsigned int );
extern void polygon_add_contour( struct polygon *, struct contour * );
extern struct polygon * new_polygon( void );
extern void free_polygon( struct polygon * );
extern void polygon_compute_holes( struct polygon * );
#ifdef DEBUG
extern void _dump_polygon( struct polygon * );
#endif // DEBUG

// Sweep event functions
extern struct segment * sweep_event_get_segment( struct sweep_event * );
extern bool sweep_event_ev_comp( struct sweep_event *, struct sweep_event * );
extern bool sweep_event_sl_comp( struct sweep_event *, struct sweep_event * );
extern bool sweep_event_ev_comp_wrapper( void *, void * );
extern bool sweep_event_sl_comp_wrapper_inverted( void *, void * );
extern bool sweep_event_sl_comp_wrapper( void *, void * );
extern bool sweep_event_below( struct sweep_event *, Point * );
extern bool sweep_event_above( struct sweep_event *, Point * );
extern bool sweep_event_ev_segment_comp(
    struct sweep_event *,
    struct sweep_event *
);

extern bool sweep_event_sl_segment_comp_wrapper_inverted( void *, void * );
extern bool sweep_event_sl_segment_comp_wrapper( void *, void * );
extern bool sweep_event_sl_segment_comp(
    struct sweep_event *,
    struct sweep_event *
);

extern struct sweep_event * new_sweep_event( void );
extern void free_sweep_event( struct sweep_event * );
#ifdef DEBUG
extern void _dump_sweep_event( struct sweep_event * );
extern void _dump_sweep_event_dlpq_wrapper( void * );
#endif // DEBUG
extern bool sweep_event_equal( struct sweep_event *, struct sweep_event * );

// Set and Buffer maintenance functions
extern struct sweep_event ** _manage_ev_buffer(
    struct sweep_event **,
    unsigned int
);

extern void _sort_ev_buffer(
    struct sweep_event **,
    unsigned int,
    unsigned int
);

extern struct sweep_event ** _process_ev_buffer(
    struct sweep_event **,
    unsigned int
);

extern unsigned int _se_set_insert(
    struct sweep_event ***,
    struct sweep_event *,
    unsigned int *,
    bool (*)( struct sweep_event *, struct sweep_event * )
);

extern void _se_set_remove(
    struct sweep_event ***,
    unsigned int,
    unsigned int *
);

#ifdef DEBUG
extern void _dump_se_set( struct sweep_event ***, unsigned int );
#endif // DEBUG
#endif // POLYGON_H
