/*------------------------------------------------------------------------------
 * connector.h
 *     Header file for polygon connector array (polygon_connector) and point
 *     chain (connector) functions and utilities.
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      connector.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "postgres.h"
#include "utils/geo_decls.h"

#include "segment.h"
#include "util.h"
#include "polygon.h"

struct connector {
    Point ** list;
    bool _closed;
    unsigned int length;
};

struct polygon_connector {
    struct connector ** open;
    unsigned int open_length;
    struct connector ** closed;
    unsigned int closed_length;
};

// Point Connector Functions
extern struct connector * new_connector( Point * );
extern void free_connector( struct connector * );
extern void connector_add_point( struct connector *, Point * );
extern bool connector_link_segment( struct connector *, struct segment * );
extern void connector_push_front( struct connector *, Point * );
extern bool connector_link_chain( struct connector *, struct connector * );

extern void connector_splice(
    struct connector *,
    unsigned int,
    struct connector *,
    unsigned int
);

extern void connector_reverse( struct connector * );
extern void connector_pop_front( struct connector * );
extern void connector_pop( struct connector * );
#ifdef DEBUG
extern void _dump_connector( struct connector * );
#endif // DEBUG


// Polygon Connector Functions
extern struct polygon_connector * new_polygon_connector(
    struct connector *,
    struct connector *
);

extern void free_polygon_connector( struct polygon_connector * );

extern void polygon_connector_add_open_connector(
    struct polygon_connector *,
    struct connector *
);

extern void polygon_connector_add_closed_connector(
    struct polygon_connector *,
    struct connector *
);

extern void polygon_connector_add_segment(
    struct polygon_connector *,
    struct segment *
);

extern void polygon_connector_remove_closed_connector(
    struct polygon_connector *,
    unsigned int
);

extern void polygon_connector_remove_open_connector(
    struct polygon_connector *,
    unsigned int
);

extern struct polygon * polygon_connector_to_polygon(
    struct polygon_connector *
);

#ifdef DEBUG
extern void _dump_polygon_connector( struct polygon_connector * );
#endif // DEBUG

#endif // CONNECTOR_H
