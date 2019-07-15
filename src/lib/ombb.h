/*------------------------------------------------------------------------------
 * ombb.h
 *      Oriented minimum bounding box prototypes and structures
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      ombb.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef OMBB_H
#define OMBB_H

#include <math.h>
#include "postgres.h"
#include "utils/geo_decls.h"
#include "util.h"
#include "poly_funcs.h"

struct ombb_solution {
    double angle;
    double area;
    double height;
    double width;
    Point  points[4];
};

extern Point ** get_ombb( POLYGON *, double *, double *, double *, bool );

extern void get_aa_bounding_box( LSEG *, unsigned int, double *, double *, double * );
extern void rotate_edge_list( LSEG *, unsigned int, Point *, double );
extern void rotate_point_about_center( Point *, Point *, double );

#endif // OMBB_H
