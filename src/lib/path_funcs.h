/*------------------------------------------------------------------------------
 * path_funcs.h
 *      Helper functions of pgpolybool path functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      path_funcs.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef PATH_FUNCS_H
#define PATH_FUNCS_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include <math.h>
#include "util.h"
#include "box_funcs.h"

extern Point ** get_path_points( PATH * );
extern LSEG ** get_path_lsegs( PATH * );

#endif // PATH_FUNCS_H
