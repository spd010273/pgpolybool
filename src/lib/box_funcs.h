/*------------------------------------------------------------------------------
 * box_funcs.h
 *      Header File for pgpolybool BOX functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      box_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef BOX_FUNCS_H
#define BOX_FUNCS_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "math.h"

extern Point ** get_box_points( BOX * );
extern LSEG ** get_box_lsegs( BOX * );

#endif // BOX_FUNCS_H
