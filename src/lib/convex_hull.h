/*------------------------------------------------------------------------------
 * convex_hull.h
 *     certainly be improved upon.
 *
 * Copyright (c) 2019, Nead Werx, Inc.
 * Copyright (c) 2019, Chris Autry
 *
 * IDENTIFICATION
 *      convex_hull.h
 *
 *------------------------------------------------------------------------------
 */

#include "util.h"

typedef enum {
    right_turn,
    left_turn,
    inline_turn
} turn;

POLYGON * get_convex_hull( Point **, unsigned int );

turn get_turn_type( Point *, Point *, Point * );
Point * get_lowest_point( Point **, unsigned int );
double get_point_angle( Point *, Point * );


// Mergesort implementation by angle to the lowest reference

void sort_points( Point **, unsigned int, unsigned int, unsigned int );
void merge( Point **, unsigned int, unsigned int, unsigned int, unsigned int );
