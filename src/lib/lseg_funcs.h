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

extern bool line_segment_intersect( LSEG *, LSEG * );
extern double line_segment_distance( LSEG *, LSEG * ); 
extern Point * line_segment_intersection( LSEG *, LSEG * );
extern LSEG ** line_segment_parallel_line_segment( LSEG *, Point *, double );
extern LSEG ** line_segment_orthogonal_line_segment( LSEG *, Point *, double );

#endif // LSEG_FUNCS_H
