#ifndef MARTINEZ_H
#define MARTINEZ_H

#include "segment.h"
#include "contour.h"
#include "polygon.h"
#include "pqueue.h"
#include "connector.h"
#include "util.h"

#define OP_INTERSECTION 0
#define OP_UNION 1
#define OP_DIFFERENCE 2
#define OP_XOR 3

#define EDGE_TYPE_NORMAL 0
#define EDGE_TYPE_NON_CONTRIBUTING 1
#define EDGE_TYPE_SAME_TRANSITION 2
#define EDGE_TYPE_DIFFERENT_TRANSITION 3

#define POLY_TYPE_SUBJECT 0
#define POLY_TYPE_CLIPPING 1

extern void process_segment( struct segment *, int, struct pqueue_node **, struct sweep_event ***, int * );
extern void compute( struct polygon *, struct polygon *, int, struct polygon * );
extern void possible_intersection( struct sweep_event *, struct sweep_event *, int *, struct pqueue_node ** );
extern void divide_segment( struct sweep_event *, Point *, struct pqueue_node ** );

extern POLYGON * mpoly_to_poly( struct polygon * );
extern struct polygon * poly_to_mpoly( POLYGON * );
extern void free_pgpoly( POLYGON * );

#endif
