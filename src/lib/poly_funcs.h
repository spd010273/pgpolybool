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
#include "lseg_funcs.h"

#define DEFAULT_CIRCLE_POLY_POINTS 12
#define DEFAULT_CIRCLE_POLY_RADIUS 1.0

extern void set_polygon_boundbox( POLYGON * );
extern double get_polygon_area( POLYGON * );
extern void rotate_polygon( POLYGON *, double );
extern Point ** get_polygon_points( POLYGON * );
extern bool polygon_contains_point( POLYGON *, Point * );
extern POLYGON * box_to_polygon( BOX * );
extern POLYGON * line_segment_to_polygon( LSEG *, double );
extern POLYGON * polygon_from_points( Point **, unsigned int );
extern LSEG ** get_polygon_lsegs( POLYGON * );
extern double get_polygon_distance( POLYGON *, POLYGON * );
#ifdef DEBUG
extern void dump_polygon( POLYGON * );
#endif // DEBUG

#endif // POLY_FUNCS_H
