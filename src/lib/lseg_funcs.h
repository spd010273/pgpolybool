/*------------------------------------------------------------------------------
 * lseg_funcs.h
 *      Header File for pgpolybool LSEG functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      lseg_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef LSEG_FUNCS_H
#define LSEG_FUNCS_H

#include "postgres.h"
#include "martinez.h"
#include "utils/geo_decls.h"
#include "poly_funcs.h"
#include "point_funcs.h"
#include "box_funcs.h"
#include "path_funcs.h"
#include "util.h"

extern bool line_segment_intersect( LSEG *, LSEG * );
extern Point * line_segment_intersection( LSEG *, LSEG * );
extern double line_segment_distance( LSEG *, LSEG * );
extern LSEG ** line_segment_parallel_line_segment( LSEG *, Point *, double );
extern LSEG ** line_segment_orthogonal_line_segment( LSEG *, Point *, double );
extern LSEG * scale_lseg( LSEG *, double, Point * );
extern double get_angle_of_lseg_intersection( LSEG *, LSEG * );
extern Point * lseg_to_vector( LSEG * );
extern double cross_product( Point *, Point * );
extern bool lseg_points_right_of( LSEG *, LSEG * );
extern bool lseg_points_left_of( LSEG *, LSEG * );
extern LSEG * get_polygon_line_segments( POLYGON * );
extern LSEG * get_root_orthogonal_segment( LSEG *, Point *, double );

extern LSEG * polygon_to_lseg( POLYGON * );
extern LSEG * point_to_lseg( Point * );
extern LSEG * line_to_lseg( LINE * );
extern LSEG * path_to_lseg( PATH * );
extern LSEG * circle_to_lseg( CIRCLE * );

#endif // LSEG_FUNCS_H
