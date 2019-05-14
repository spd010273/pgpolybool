/*------------------------------------------------------------------------------
 * point_funcs.h
 *      Helper functions of pgpolybool point functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      point_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef POINT_FUNCS_H
#define POINT_FUNCS_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "util.h"
#include <math.h>

extern Point * line_to_point( LINE * );
extern Point * path_to_point( PATH * );

extern LINE * best_fit_line( Point *, unsigned int );

#endif // POINT_FUNCS_H
