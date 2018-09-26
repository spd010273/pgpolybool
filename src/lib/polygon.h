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
    int polygon;
    struct sweep_event * other;
    bool in_out;
    int position;
    int edge_type;
    int polygon_type;
};

// Polygon functions
int polygon_num_points( struct polygon * );
void polygon_boundingbox( struct polygon *, Point *, Point * );
void polygon_move( struct polygon *, double, double );
void polygon_erase_contour( struct polygon *, int );
void polygon_add_contour( struct polygon *, struct contour * );
struct polygon * new_polygon( void );
void free_polygon( struct polygon * );
void polygon_compute_holes( struct polygon * );

// Sweep event functions
struct segment * sweep_event_get_segment( struct sweep_event * );
bool sweep_event_comp( struct sweep_event *, struct sweep_event * );
bool sweep_event_below( struct sweep_event *, Point * );
bool sweep_event_above( struct sweep_event *, Point * );
bool sweep_event_segment_comp( struct sweep_event *, struct sweep_event * );
struct sweep_event * new_sweep_event( void );
void free_sweep_event( struct sweep_event * );

// Set and Buffer maintenance functions
struct sweep_event ** _manage_ev_buffer( struct sweep_event **, int );
void _sort_ev_buffer( struct sweep_event **, int, int );
struct sweep_event ** _process_ev_buffer( struct sweep_event **, int );
int _se_set_insert( struct sweep_event ***, struct sweep_event *, int );
void _se_set_remove( struct sweep_event ***, int, int );
//void _se_push_front( struct sweep_event ***, int *, struct sweep_event * );

#endif
