#include "connector.h"

struct connector * new_connector( Point * p )
{
    struct connector * c = NULL;
    Point ** list        = NULL;

    c = ( struct connector * ) palloc0( sizeof( struct connector ) );

    c->_closed = false;
    c->length = 1;

    list = ( Point ** ) palloc0( sizeof( Point * ) );

    c->list = list;
    c->list[0] = p;

    return c;
}

void free_connector( struct connector * c )
{
    if( c == NULL )
    {
        return;
    }

    if( c->list != NULL )
    {
        pfree( c->list );
    }

    pfree( c );
    return;
}

void connector_add_point( struct connector * c, Point * p )
{
    if( p == NULL )
    {
        return;
    }

    c->list = ( Point ** ) repalloc(
        c->list,
        sizeof( Point * )
      * ( c->length + 1 )
    );
    c->list[c->length] = p;
    c->length++;

    return;
}

void connector_push_front( struct connector * c, Point * p )
{
    int i = 0;

    if( c == NULL || p == NULL )
    {
        return;
    }

    if( c->length == 0 )
    {
        connector_add_point( c, p );
        return;
    }

    c->list = ( Point ** ) repalloc( c->list, sizeof( Point * ) * ( c->length + 1 ) );

    for( i = c->length; i > 0; i-- )
    {
        c->list[i] = c->list[i-1];
    }

    c->list[0] = p;
    c->length++;
    return;
}

bool connector_link_segment( struct connector * c, struct segment * s )
{
    if( s == NULL || c == NULL )
    {
        return false;
    }

    if( c->length == 0 || c->list == NULL )
    {
        elog( DEBUG1, "Could not link segment: point list is uninitialized" );
        return false;
    }

    if( points_equal( s->p1, c->list[0] ) )
    {
        if( points_equal( s->p2, c->list[c->length - 1] ) )
        {
            c->_closed = true;
        }
        else
        {
            connector_push_front( c, s->p2 );
        }

        return true;
    }

    if( points_equal( s->p2, c->list[c->length - 1] ) )
    {
        if( points_equal( s->p1, c->list[0] ) )
        {
            c->_closed = true;
        }
        else
        {
            connector_add_point( c, s->p1 );
        }

        return true;
    }

    if( points_equal( s->p2, c->list[0] ) )
    {
        if( points_equal( s->p1, c->list[c->length - 1] ) )
        {
            c->_closed = true;
        }
        else
        {
            connector_push_front( c, s->p1 );
        }

        return true;
    }

    if( points_equal( s->p1, c->list[c->length - 1] ) )
    {
        if( points_equal( s->p2, c->list[0] ) )
        {
            c->_closed = true;
        }
        else
        {
            connector_add_point( c, s->p2 );
        }

        return true;
    }

    return false;
}

bool connector_link_chain( struct connector * c0, struct connector * c1 )
{
    if( c0 == NULL || c1 == NULL )
    {
        return false;
    }

    _dump_connector( c0 );
    _dump_connector( c1 );
    if( points_equal( c1->list[0], c0->list[c0->length - 1] ) )
    {
        connector_pop_front( c1 );
        connector_splice( c0, c0->length, c1, 0 );
        return true;
    }

    if( points_equal( c1->list[c1->length - 1], c0->list[0] ) )
    {
        connector_pop_front( c0 );
        connector_splice( c0, 0, c1, 0 );
        return true;
    }

    if( points_equal( c1->list[0], c0->list[0] ) )
    {
        connector_pop_front( c0 );
        connector_reverse( c1 );
        connector_splice( c0, 0, c1, 0 );
        return true;
    }

    if( points_equal( c1->list[c1->length - 1], c0->list[c0->length - 1] ) )
    {
        connector_pop( c0 );
        connector_reverse( c1 );
        connector_splice( c0, c0->length, c1, 0 );
        return true;
    }

    return false;
}

void connector_splice( struct connector * c_to, int ins_index, struct connector * c_from, int start_ind )
{
    Point ** list_temp = NULL;
    int temp_len = 0;
    int i = 0;
    int to_ind = 0;

    if(
           c_to == NULL
        || c_from == NULL
        || ins_index > c_to->length
        || start_ind >= c_from->length
      )
    {
        return;
    }

    temp_len = c_to->length - ins_index;

    if( temp_len > 0 )
    {
        list_temp = ( Point ** ) palloc0( sizeof( Point * ) * temp_len );

        for( i = ins_index; i < c_to->length; i++ )
        {
            list_temp[i - ins_index] = c_to->list[i];
        }
    }

    c_to->list = ( Point ** ) repalloc(
        c_to->list,
        sizeof( Point * )
      * ( c_to->length + ( c_from->length - start_ind ) )
    );

    to_ind = ins_index;

    for( i = start_ind; i < c_from->length; i++ )
    {
        c_to->list[to_ind] = c_from->list[i];
        to_ind++;
    }

    if( temp_len > 0 )
    {
        for( i = 0; i < temp_len; i++ )
        {
            c_to->list[to_ind] = list_temp[i];
            to_ind++;
        }

        pfree( list_temp );
    }

    c_to->length = c_to->length + ( c_from->length - start_ind );

    if( start_ind == 0 )
    {
        pfree( c_from->list );
        c_from->length = 0;
    }
    else
    {
        list_temp = ( Point ** ) palloc0( sizeof( Point * ) * ( c_from->length - start_ind ) );
        to_ind = 0;
        for( i = 0; i < start_ind; i++ )
        {
            list_temp[to_ind] = c_from->list[i];
            to_ind++;
        }

        pfree( c_from->list );
        c_from->list = list_temp;
        c_from->length = to_ind;
    }

    return;
}

void connector_reverse( struct connector * c )
{
    Point * temp = NULL;
    int i = 0;

    if( c == NULL || c->length == 0 )
    {
        return;
    }

    for( i = 0; i < ( int ) ( c->length / 2 ); i++ )
    {
        temp = c->list[i];
        c->list[i] = c->list[c->length - 1 - i];
        c->list[c->length - 1 - i] = temp;
    }

    return;
}

void connector_pop_front( struct connector * c )
{
    Point ** list_temp = NULL;
    int i = 0;

    if( c == NULL || c->length == 0 )
    {
        return;
    }

    if( c->length == 1 )
    {
        c->length = 0;
        pfree( c->list );
        return;
    }

    list_temp = ( Point ** ) palloc0( sizeof( Point * ) * ( c->length - 1 ) );

    for( i = 0; i < c->length - 1; i++ )
    {
        list_temp[i] = c->list[i + 1];
    }

    pfree( c->list );
    c->list = list_temp;
    c->length = c->length - 1;
    return;
}

void connector_pop( struct connector * c )
{
    Point ** list_temp = NULL;
    int i = 0;

    if( c == NULL || c->length == 0 )
    {
        return;
    }

    if( c->length == 1 )
    {
        c->length = 0;
        pfree( c->list );
        return;
    }

    list_temp = ( Point ** ) palloc0( sizeof( Point * ) * ( c->length - 1 ) );

    for( i = 0; i < c->length - 1; i++ )
    {
        list_temp[i] = c->list[i];
    }

    pfree( c->list );
    c->list = list_temp;
    c->length = c->length - 1;
    return;
}

struct polygon_connector * new_polygon_connector( struct connector * open, struct connector * closed )
{
    struct polygon_connector * new_pc = NULL;

    new_pc = ( struct polygon_connector * ) palloc0( sizeof( struct polygon_connector ) );
    new_pc->open_length = 0;
    new_pc->closed_length = 0;

    if( open != NULL )
    {
        new_pc->open = ( struct connector ** ) palloc0( sizeof( struct connector * ) );
        new_pc->open[0] = open;
        new_pc->open_length = 1;
    }

    if( closed != NULL )
    {
        new_pc->closed = ( struct connector ** ) palloc0( sizeof( struct connector * ) );
        new_pc->closed[0] = closed;
        new_pc->closed_length = 1;
    }

    return new_pc;
}

void free_polygon_connector( struct polygon_connector * pc )
{
    if( pc == NULL )
    {
        return;
    }

    if( pc->open != NULL )
    {
        pfree( pc->open );
    }

    if( pc->closed != NULL )
    {
        pfree( pc->closed );
    }

    pfree( pc );
    return;
}

void polygon_connector_add_open_connector( struct polygon_connector * pc, struct connector * c )
{
    if( pc == NULL || c == NULL )
    {
        return;
    }

    if( pc->open == NULL )
    {
        pc->open = ( struct connector ** ) palloc0(
            sizeof( struct connector * )
        );
        pc->open[0] = c;
        pc->open_length = 1;
    }
    else
    {
        pc->open = ( struct connector ** ) repalloc(
            pc->open,
            sizeof( struct connector * )
          * ( pc->open_length + 1 )
        );
        pc->open[pc->open_length] = c;
        pc->open_length++;
    }

    return;
}

void polygon_connector_add_closed_connector( struct polygon_connector * pc, struct connector * c )
{
    if( pc == NULL || c == NULL )
    {
        return;
    }

    if( pc->closed == NULL )
    {
        pc->closed = ( struct connector ** ) palloc0(
            sizeof( struct connector * )
        );
        pc->closed[0] = c;
        pc->closed_length = 1;
    }
    else
    {
        pc->closed = ( struct connector ** ) repalloc(
            pc->closed,
            sizeof( struct connector * )
          * ( pc->closed_length + 1 )
        );
        pc->closed[pc->closed_length] = c;
        pc->closed_length++;
    }
    return;
}

void polygon_connector_remove_open_connector( struct polygon_connector * pc, int index )
{
    struct connector ** list_temp = NULL;
    int i = 0;
    int ind_offset = 0;

    if( pc == NULL || index >= pc->open_length || index < 0 )
    {
        return;
    }

    list_temp = ( struct connector ** ) palloc0(
        sizeof( struct connector * )
      * ( pc->open_length - 1 )
    );

    for( i = 0; i < pc->open_length - 1; i++ )
    {
        if( i == index )
        {
            ind_offset++;
        }

        list_temp[i] = pc->open[i + ind_offset];
    }

    pfree( pc->open );
    pc->open = list_temp;
    pc->open_length--;
    return;
}

void polygon_connector_remove_closed_connector( struct polygon_connector * pc, int index )
{
    struct connector ** list_temp = NULL;
    int i = 0;
    int ind_offset = 0;

    if( pc == NULL || index >= pc->closed_length || index < 0 )
    {
        return;
    }

    list_temp = ( struct connector ** ) palloc0(
        sizeof( struct connector * )
      * ( pc->closed_length - 1 )
    );

    for( i = 0; i < pc->closed_length - 1; i++ )
    {
        if( i == index )
        {
            ind_offset++;
        }

        list_temp[i] = pc->closed[i + ind_offset];
    }

    pfree( pc->closed );
    pc->closed = list_temp;
    pc->closed_length--;
    return;
}


void polygon_connector_add_segment( struct polygon_connector * pc, struct segment * s )
{
    int i                   = 0;
    int k                   = 0;
    struct connector * temp = NULL;

    if( pc == NULL || s == NULL )
    {
        elog( DEBUG1, "Could not add segment: pc %p, s %p", pc, s );
        return;
    }

    while( i != pc->open_length )
    {
        if( connector_link_segment( pc->open[i], s ) )
        {
            if( pc->open[i]->_closed )
            {
                polygon_connector_add_closed_connector( pc, pc->open[i] );
                polygon_connector_remove_open_connector( pc, i );
            }
            else
            {
                k = i;

                for( ++k; k < pc->open_length; k++ )
                {
                    if( connector_link_chain( pc->open[i], pc->open[k] ) )
                    {
                        polygon_connector_remove_open_connector( pc, k );
                        break;
                    }
                }
            }

            return;
        }

        i++;
    }

    temp = new_connector( s->p1 );
    connector_add_point( temp, s->p2 );
    polygon_connector_add_open_connector( pc, temp );
    return;
}

struct polygon * polygon_connector_to_polygon( struct polygon_connector * pc )
{
    struct polygon * p = NULL;
    struct contour * c = NULL;
    int              i = 0;
    int              j = 0;

    elog( DEBUG1, "Entry, polygon_connector_to_polygon" );
    if( pc == NULL || pc->closed_length == 0 )
    {
        return NULL;
    }

    p = new_polygon();

    for( i = 0; i < pc->closed_length; i++ )
    {
        if( pc->closed[i]->length > 0 )
        {
            c = new_contour();

            for( j = 0; j < pc->closed[i]->length; j++ )
            {
                contour_add_point( c, pc->closed[i]->list[j] );
            }

            polygon_add_contour( p, c );
        }
    }

    return p;
}

void _dump_connector( struct connector * c )
{
    int i = 0;

    if( c == NULL )
    {
        elog( DEBUG1, "Connector is null!" );
        return;
    }

    elog(
        DEBUG1,
        "Connector: %p\n    list: %p\n    _closed: %s\n    length: %d",
        c,
        c->list,
        c->_closed ? "true" : "false",
        c->length
    );

    if( c->list == NULL )
    {
        return;
    }

    for( i = 0; i < c->length; i++ )
    {
        elog( DEBUG1, "list[%d]: (%f, %f)", i, c->list[i]->x, c->list[i]->y );
    }

    return;
}

void _dump_polygon_connector( struct polygon_connector * pc )
{
    int i = 0;
    int j = 0;
    struct connector * c = NULL;

    if( pc == NULL )
    {
        elog( DEBUG1, "Polygon connector is null" );
        return;
    }

    elog( DEBUG1, "Polygon connector ADDR %p", ( void * ) pc );
    elog( DEBUG1, "pc->open_length: %d", pc->open_length );
    elog( DEBUG1, "pc->open: %p", pc->open );
    for( i = 0; i < pc->open_length; i++ )
    {
        c = pc->open[i];

        if( c == NULL )
        {
            elog( DEBUG1, "pc->open[%d] NULL", i );
        }
        else
        {
            elog(
                DEBUG1,
                "pc->open[%d] %p\n" \
                "            list: %p\n" \
                "            _closed: %s\n" \
                "            length: %d",
                i,
                c,
                c->list,
                c->_closed?"true":"false",
                c->length
            );

            if( c->list != NULL )
            {
                for( j = 0; j < c->length; j++ )
                {
                    elog(
                        DEBUG1,
                        "                [%d] ( %f, %f )",
                        j,
                        c->list[j]->x,
                        c->list[j]->y
                    );
                }
            }
        }
    }
    
    elog( DEBUG1, "pc->closed_length: %d", pc->closed_length );
    elog( DEBUG1, "pc->closed %p", pc->closed );

    for( i = 0; i < pc->closed_length; i++ )
    {
        c = pc->closed[i];

        if( c == NULL )
        {
            elog( DEBUG1, "pc->closed[%d] NULL", i );
        }
        else
        {
            elog(
                DEBUG1,
                "pc->closed[%d] %p\n" \
                "            list: %p\n" \
                "            _closed: %s\n" \
                "            length: %d",
                i,
                c,
                c->list,
                c->_closed?"true":"false",
                c->length
            );

            if( c->list != NULL )
            {
                for( j = 0; j < c->length; j++ )
                {
                    elog(
                        DEBUG1,
                        "                [%d] ( %f, %f )",
                        j,
                        c->list[j]->x,
                        c->list[j]->y
                    );
                }
            }
        }
    }
    return;
}
