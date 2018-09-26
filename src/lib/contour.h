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

void contour_change_orientation( struct contour * );
void contour_erase_point( struct contour *, int );
void contour_set_clockwise( struct contour * );
void contour_set_counterclockwise( struct contour * );
bool contour_counterclockwise( struct contour * );
bool contour_clockwise( struct contour * );
void contour_bounding_box( struct contour *, Point *, Point * );
void contour_add_hole( struct contour *, int );
void contour_add_point( struct contour *, Point * );
void contour_set_external( struct contour *, bool );
struct segment * contour_get_segment( struct contour *, int );
struct contour * new_contour( void );
void free_contour( struct contour * );
double contour_area( struct contour * );

#endif
