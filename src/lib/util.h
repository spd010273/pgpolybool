/*
 * pgpolybool is released under the PostgreSQL license.
 * 
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written agreement is
 * hereby granted, provided that the above copyright notice and this paragraph and
 * the following two paragraphs appear in all copies.
 * 
 * IN NO EVENT SHALL Nead Werx, Inc. or Chris Autry BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
 * OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF Nead Werx, Inc. or Chris Autry HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Nead Werx, Inc. and Chris Autry SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER
 * Nead Werx, Inc. or Chris Autry HAVE NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef UTIL_H
#define UTIL_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "segment.h"
#include <math.h>

#define FP_FUDGE_FACTOR 128
#define DBL_EPSILON (2.2204460492503131e-16) * FP_FUDGE_FACTOR
#define DBL_MAX (1.79769e+308)

extern double signed_area_three( Point *, Point *, Point * );
extern double signed_area_two( Point *, Point * );
extern int sign( Point *, Point *, Point * );
extern bool point_in_triangle( struct segment *, Point *, Point * );
extern double distance( Point *, Point * );

extern unsigned int find_intersection(
    struct segment *,
    struct segment *,
    Point *,
    Point *
);

extern bool points_equal( Point *, Point * );
#endif
