/*------------------------------------------------------------------------------
 * polyprocessing.h
 *     Header file for polygon frontend and backend processing functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      polyprocessing.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef POLYPROCESSING_H
#define POLYPROCESSING_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "fmgr.h"

#include "util.h"

#define ZOOM_RATE 1.04

extern POLYGON ** poly_preprocessing_array(
    ArrayType *,
    bool,
    bool,
    Point ***,
    unsigned int *
);

extern POLYGON * poly_preprocessing( POLYGON *, bool, Point ** );
extern POLYGON * poly_postprocessing( POLYGON *, Point **, unsigned int, bool );
extern bool points_colinear( Point *, Point *, Point * );
extern void remove_colinear_point( POLYGON **, Point * );
extern double get_polygon_area( POLYGON * );
#ifdef DEBUG
extern void dump_polygon( POLYGON * );
#endif // DEBUG

#endif // POLYPROCESSING_H
