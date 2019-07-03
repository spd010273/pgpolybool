/*------------------------------------------------------------------------------
 * convex_hull.h
 *     convex hull prototypes implementing Graham Scan
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      convex_hull.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include "util.h"

// To ease interpretation of the double output of get_turn_type, which
// is just a ccw function
typedef enum {
    right_turn,
    left_turn,
    inline_turn
} turn;

POLYGON * get_convex_hull( Point **, unsigned int );

turn get_turn_type( Point *, Point *, Point * );
Point * get_lowest_point( Point **, unsigned int );
double get_point_angle( Point *, Point * ); // Second argument is the lowest point

// Mergesort implementation by angle to the lowest reference
void sort_points( Point **, unsigned int, unsigned int, unsigned int );
void merge( Point **, unsigned int, unsigned int, unsigned int, unsigned int );

#endif // CONVEX_HULL_H
