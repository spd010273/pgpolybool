/*------------------------------------------------------------------------------
 * util.h
 *     Header file for point math and helper functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      util.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef UTIL_H
#define UTIL_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "segment.h"
#include <math.h>

#define PGPOLYBOOL_VERSION "1.0"

#define FP_FUDGE_FACTOR 4096

#ifndef DBL_EPSILON
#define DBL_EPSILON (2.2204460492503131e-16) * FP_FUDGE_FACTOR
#endif // DBL_EPSILON

#ifndef DBL_MAX
#define DBL_MAX (1.79769e+308)
#endif // DBL_MAX

#ifndef PI
#define PI (3.14159265358979)
#endif // PI

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
extern double dot_product( Point *, Point * );
#endif // UTIL_H
