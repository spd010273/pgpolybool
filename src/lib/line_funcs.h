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

extern LINE ** line_parallel_line( LINE *, Point *, double );
extern LINE * line_orthogonal_line( LINE *, Point * );
extern double get_angle_of_line_intersection( LINE *, LINE * );

#endif // LINE_FUNCS_H
