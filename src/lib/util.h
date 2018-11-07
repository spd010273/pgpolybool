#ifndef UTIL_H
#define UTIL_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "segment.h"
#include <math.h>

#define FP_FUDGE_FACTOR 128
#define DBL_EPSILON (2.2204460492503131e-16) * FP_FUDGE_FACTOR
#define DBL_MAX (1.79769e+308)

extern double signed_area_three( Point *, Point *, Point * );
extern double signed_area_two( Point *, Point * );
extern int sign( Point *, Point *, Point * );
extern bool point_in_triangle( struct segment *, Point *, Point * );
extern double distance( Point *, Point * );

extern unsigned int find_intersection(
    struct segment *,
    struct segment *,
    Point *,
    Point *
);

extern bool points_equal( Point *, Point * );
#endif
