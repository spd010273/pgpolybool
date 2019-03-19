/*------------------------------------------------------------------------------
 * poly_funcs.h
 *      Helper functions of pgpolybool polygon functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      poly_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef POLY_FUNCS_H
#define POLY_FUNCS_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include <math.h>
#include "util.h"
#include "box_funcs.h"

extern void set_polygon_boundbox( POLYGON * );
extern double get_polygon_area( POLYGON * );
extern void rotate_polygon( POLYGON *, double );
extern Point ** get_polygon_points( POLYGON * );
extern POLYGON * box_to_polygon( BOX * );
#ifdef DEBUG
extern void dump_polygon( POLYGON * );
#endif // DEBUG

#endif // POLY_FUNCS_H
