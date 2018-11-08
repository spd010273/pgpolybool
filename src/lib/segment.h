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

#ifndef SEGMENT_H
#define SEGMENT_H

#include "postgres.h"
#include "utils/geo_decls.h"

struct segment {
    Point * p1; // begin
    Point * p2; // end
};

extern void segment_set_begin( struct segment *, Point * p );
extern void segment_set_end( struct segment *, Point * p );
extern void segment_change_orientation( struct segment * );
extern struct segment * new_segment( void );
extern void free_segment( struct segment * );

#endif
