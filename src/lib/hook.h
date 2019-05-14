/*------------------------------------------------------------------------------
 * hook.h
 *      Function hooking declarations
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      hook.h
 *
 *------------------------------------------------------------------------------
 */

#ifndef HOOK_H
#define HOOK_H

#include "postgres.h"
#include <stdint.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
//#include <stdbool.h> // conflicts with postgresql typing

#define __ALLOC(size) palloc0(size)
#define __FREE(ptr) pfree(ptr)
#define __LOG(msg,args...) elog(DEBUG1,msg,args)
#define __ERROR(msg,args...) ereport( ERROR, ( errcode( ERRCODE_EXTERNAL_ROUTINE_EXCEPTION ), errmsg( msg, args ) ) )

extern bool hook_function( char *, uintptr_t );

#endif // HOOK_H
