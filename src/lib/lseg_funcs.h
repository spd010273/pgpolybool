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
#include "util.h"

extern double line_segment_distance( LSEG *, LSEG * ); 
extern LSEG ** line_segment_parallel_line_segment( LSEG *, Point *, double );
extern LSEG ** line_segment_orthogonal_line_segment( LSEG *, Point *, double );
extern LSEG * scale_lseg( LSEG *, double, Point * );
extern double get_angle_of_intersection( LSEG *, LSEG * );
extern Point * lseg_to_vector( LSEG * );
extern double cross_product( Point *, Point * );
extern bool lseg_points_right_of( LSEG *, LSEG * );
extern bool lseg_points_left_of( LSEG *, LSEG * );

#endif // LSEG_FUNCS_H
