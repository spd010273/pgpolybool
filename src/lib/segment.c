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
