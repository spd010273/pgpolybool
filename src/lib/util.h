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

#define signed_area_three( a, b, c ) (((a->x - c->x) * (b->y - c->y) - (b->x - c->x) * (a->y - c->y)))
#define signed_area_two( a, b ) (((-b->x * (a->y - b->y)) - (-b->y * (a->x - b->x))))
#define distance( a, b ) ((sqrt((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y))))
#define points_equal( a, b ) ((!(a == NULL || b == NULL) && a->x == b->x && a->y == b->y))
#define dot_product( a, b ) ((a->x * b->x + a->y * b->y))
#define cross_product( a, b ) ((a->x * b->y - a->y * b->x))

extern int sign( Point *, Point *, Point * );
extern bool point_in_triangle( struct segment *, Point *, Point * );

extern unsigned int find_intersection(
    struct segment *,
    struct segment *,
    Point *,
    Point *
);
extern void __oom( const char * );
extern void __degenerate_solution( double, double, double );
#endif // UTIL_H
