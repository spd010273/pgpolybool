#ifndef CONTOUR_H
#define CONTOUR_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "segment.h"
#include "util.h"

struct contour {
    Point ** points;
    int   num_points;
    int * holes;
    int   num_holes;
    bool _external;
    bool _precomputed_cc;
    bool _cc;
};

extern void contour_change_orientation( struct contour * );
extern void contour_erase_point( struct contour *, int );
extern void contour_set_clockwise( struct contour * );
extern void contour_set_counterclockwise( struct contour * );
extern bool contour_counterclockwise( struct contour * );
extern bool contour_clockwise( struct contour * );
extern void contour_bounding_box( struct contour *, Point *, Point * );
extern void contour_add_hole( struct contour *, int );
extern void contour_add_point( struct contour *, Point * );
extern void contour_set_external( struct contour *, bool );
extern struct segment * contour_get_segment( struct contour *, int );
extern struct contour * new_contour( void );
extern void free_contour( struct contour * );
extern double contour_area( struct contour * );
extern void _dump_contour( struct contour * );
#endif
