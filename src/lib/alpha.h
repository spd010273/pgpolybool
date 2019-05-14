/*------------------------------------------------------------------------------
 * alpha.h
 *     Header file for alpha shape computation helper functions
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      alpha.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef ALPHA_H
#define ALPHA_H

#include "postgres.h"
#include "utils/geo_decls.h"
#include "util.h"
#include <math.h>

bool get_alpha_shape( Point **, unsigned int, double, POLYGON **, unsigned int * );
inline unsigned long int combinations( unsigned long int, unsigned long int );
inline unsigned long int factorial( unsigned long int );

#endif // ALPHA_H 
