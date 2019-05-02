/*------------------------------------------------------------------------------
 * ombb.h
 *     Header file for oriented minimum bounding box algorithm implementing
 *     rotating calipers
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

extern Point ** get_ombb( POLYGON *, double *, double *, double *, bool );
extern inline Point _unit_vector( Point, Point );
extern inline Point _orthogonal_vector( Point );
#endif // OMBB_H
