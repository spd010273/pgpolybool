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

#ifndef CONTOUR_H
#define CONTOUR_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "segment.h"
#include "util.h"

struct contour {
    Point ** points;
    unsigned int   num_points;
    unsigned int * holes;
    unsigned int   num_holes;
    bool _external;
    bool _precomputed_cc;
    bool _cc;
};

extern void contour_change_orientation( struct contour * );
extern void contour_erase_point( struct contour *, unsigned int );
extern void contour_set_clockwise( struct contour * );
extern void contour_set_counterclockwise( struct contour * );
extern bool contour_counterclockwise( struct contour * );
extern bool contour_clockwise( struct contour * );
extern void contour_bounding_box( struct contour *, Point *, Point * );
extern void contour_add_hole( struct contour *, unsigned int );
extern void contour_add_point( struct contour *, Point * );
extern void contour_set_external( struct contour *, bool );
extern struct segment * contour_get_segment( struct contour *, unsigned int );
extern struct contour * new_contour( void );
extern void free_contour( struct contour * );
extern double contour_area( struct contour * );
#ifdef DEBUG
extern void _dump_contour( struct contour * );
#endif // DEBUG
#endif // CONTOUR_H
