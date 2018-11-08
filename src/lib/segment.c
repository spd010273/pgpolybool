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

#include "segment.h"

void segment_set_begin( struct segment * seg, Point * p )
{
    if( seg == NULL )
    {
        return;
    }

    seg->p1 = p;

    return;
}

void segment_set_end( struct segment * seg, Point * p )
{
    if( seg == NULL )
    {
        return;
    }

    seg->p2 = p;

    return;
}

void segment_change_orientation( struct segment * seg )
{
    Point * temp = NULL;

    if( seg == NULL )
    {
        return;
    }

    temp    = seg->p1;
    seg->p1 = seg->p2;
    seg->p2 = temp;

    return;
}

struct segment * new_segment( void )
{
    struct segment * new_segment = NULL;

    new_segment = ( struct segment * ) palloc0( sizeof( struct segment ) );

    if( new_segment == NULL )
    {
        ereport(
            ERROR,
            (
                errcode( ERRCODE_OUT_OF_MEMORY ),
                errmsg( "Could not create new segment" )
            )
        );
    }

    new_segment->p1 = NULL;
    new_segment->p2 = NULL;

    return new_segment;
}

void free_segment( struct segment * old_segment )
{
    if( old_segment == NULL )
    {
        return;
    }

    if( old_segment->p1 != NULL )
    {
        pfree( old_segment->p1 );
    }

    if( old_segment->p2 != NULL )
    {
        pfree( old_segment->p2 );
    }

    pfree( old_segment );
    return;
}
