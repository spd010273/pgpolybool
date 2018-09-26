#ifndef UTIL_H
#define UTIL_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "segment.h"
#include <math.h>

#define FP_FUDGE_FACTOR 128
#define DBL_EPSILON (2.2204460492503131e-16) * FP_FUDGE_FACTOR
#define DBL_MAX (1.79769e+308)

double signed_area_three( Point *, Point *, Point * );
double signed_area_two( Point *, Point * );
int sign( Point *, Point *, Point * );
bool point_in_triangle( struct segment *, Point *, Point * );
double distance( Point *, Point * );
int find_intersection( struct segment *, struct segment *, Point *, Point * );
bool points_equal( Point *, Point * );

#endif
