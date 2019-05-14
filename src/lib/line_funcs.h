/*------------------------------------------------------------------------------
 * line_funcs.h
 *      Helper functions definitions of pgpolybool line functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      line_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef LINE_FUNCS_H
#define LINE_FUNCS_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include <math.h>
#include "util.h"
#include "point_funcs.h"

extern LINE ** line_parallel_line( LINE *, Point *, double );
extern LINE * line_orthogonal_line( LINE *, Point * );
extern double get_angle_of_line_intersection( LINE *, LINE * );

extern LINE * polygon_to_line( POLYGON * );
extern LINE * point_to_line( Point * );
extern LINE * lseg_to_line( LSEG * );
extern LINE * path_to_line( PATH * );
extern LINE * box_to_line( BOX * );
extern LINE * circle_to_line( CIRCLE * );

#endif // LINE_FUNCS_H
