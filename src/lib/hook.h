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

#pragma pack( push, 1 )
struct retpoline {
    uint8_t  push_opcode;
    uint32_t address_low;
    uint8_t  mov_opcode;
    uint8_t  mov_modrm;
    uint8_t  mov_sib;
    uint8_t  mov_offset;
    uint32_t address_hi;
    uint8_t  ret_opcode;
};
#pragma pack( pop )

#define __ALLOC(size) palloc0(size)
#define __FREE(ptr) pfree(ptr)
#define __LOG(msg,args...) elog(DEBUG1,msg,args)

extern bool hook_function( char *, uintptr_t );
extern bool __hook( uintptr_t, uintptr_t );
extern uintptr_t __get_foreign_function_address( char * );

#endif // HOOK_H
