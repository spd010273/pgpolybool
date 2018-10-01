#ifndef SEGMENT_H
#define SEGMENT_H

#include "postgres.h"
#include "utils/geo_decls.h"

struct segment {
    Point * p1; // begin
    Point * p2; // end
};

extern void segment_set_begin( struct segment *, Point * p );
extern void segment_set_end( struct segment *, Point * p );
extern void segment_change_orientation( struct segment * );
extern struct segment * new_segment( void );
extern void free_segment( struct segment * );

#endif
