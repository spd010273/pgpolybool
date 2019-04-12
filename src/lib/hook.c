/*------------------------------------------------------------------------------
 * hook.c
 *      Function hooking routine
 *
 * Copyright (c) 2018, Nead Werx, Inc.
 * Copyright (c) 2018, Chris Autry
 *
 * IDENTIFICATION
 *      hook.c
 *
 *------------------------------------------------------------------------------
 */

#include "hook.h"

/* bool hook_function( char * target, uintptr_t hook )
 *
 *     Simple function hooking for x86_64 / AMD64 ISAs in Linux
 *
 *     This function attempts to locate the char * target function within the
 *     current executable's symbol table using the Real-Time Dynamic Linker,
 *     then hooks it with code that long jumps to the address provided in
 *     uintptr_t hook.
 *
 *     The long jump consists of the following:
 *
 *       PUSH immediate
 *       MOV [ESP + 4] immediate
 *       RET
 *
 *     Because this uses the a full 64-bit address, we can jump anywhere in
 *     memory using this method, and unlike others, does not dirty RAX or RSP.
 *     
 *     PUSH: 0x68: push the 32-bit immediate value onto the stack
 *     MOV:  0xC7: move a value
 *           MOV is modified (mod r/m)'s by the two following bytes:
 *           0x44: change addressing mode of MOV to register indirect
 *                 addressing using scaled index byte (SIB) with a
 *                 displacement.
 *           0x24: SIB - ESP (Stack Pointer)
 *           0x04: With offset of 4
 *     RET:  0xC3: Returns, ABI dictates the return address comes from the stack.
 *
 *     This translates to the following machine code, which is constant
 *     ( we merely shift in the high and low DWORD addresses ):
 *
 *       0x<address_00_31>68
 *       0x<address_32_63>042444C7
 *       0xC3
 *
 *     This is then packed, little-endian, into two QWORDS and written to 16-bytes,
 *     starting at the address of the target:
 *
 *       0x2444C7<address_00_31>68
 *       0x0000C3<address_31_63>04
 *
 *     In most cases, the mov [rsp + 4] is not needed (in fact, removing this
 *     yields a traditional x86 hook), but is not safe depending on what region
 *     of memory the program is loaded into.
 *
 *     NOTE: Because this overwrites 16 bytes starting at the target address,
 *     this function is NOT safe to use on targets whose size is less than 16,
 *     bytes.
 *
 *     NOTE 2: This function has been tested on systems supporting PDPE1GB
 *     (1 GB hugepages), but has not been tested on programs that are
 *     actually loaded into a hugepage.
 *
 * Arguments:
 *     char * target: Name of the target function we want to hook
 *     uintptr_t hook: Address of the function we want to run instead
 *
 * Return:
 *     Returns true if the hook is successfully installed
 *     Returns false on error.
 *
 * Error Conditions:
 *     Returns false if the symbolic table cannot be opened
 *     Returns false if the target function's address cannot be located
 *     Returns false if unable to set page permissions to write
 *
 */

bool hook_function( char * target_function, uintptr_t hooking_function )
{
    void *     program_handle   = NULL;
    uint64_t   instruction_one  = 0;
    uint64_t   instruction_two  = 0;
    uintptr_t  target_address   = 0;
    uintptr_t  page_start       = 0;
    size_t     page_size        = 0;

    if( target_function == NULL || hooking_function == 0 )
    {
        return false;
    }

    // Attempt to open the current executable's symbolic table, using the real time dynamic linker
    program_handle = dlopen( NULL, RTLD_NOW );

    if( program_handle == NULL )
    {
        return false;
    }

    // Attempt to locate the entry for the target function
    target_address = ( uintptr_t ) dlsym( program_handle, target_function );

    if( target_address == 0 )
    {
        return false;
    }

    // Determine what page the target lies in so we can update permissions to
    // allow the hook to be written
    page_size  = sysconf( _SC_PAGESIZE );
    page_start = target_address & -page_size;

    dlclose( program_handle );
    program_handle = NULL;

    // Attempt to modify permissions
    if(
        mprotect(
            ( void * ) page_start,
            ( target_address + 1 ) - page_start,
            PROT_READ | PROT_WRITE | PROT_EXEC
        ) == 0
      )
    {
        instruction_one = 0x2444C70000000068 | ( ( ( uint64_t ) ( ( uint32_t ) hooking_function ) ) << 8 );
        instruction_two = 0x0000C30000000004 | ( ( ( uint32_t ) ( hooking_function >> 32 ) ) << 8 );
        *( ( uintptr_t * )( target_address     ) ) = ( uintptr_t ) instruction_one;
        *( ( uintptr_t * )( target_address + 8 ) ) = ( uintptr_t ) instruction_two;

        // Reset page permissions back to something safer
        if(
            mprotect(
                ( void * ) page_start,
                ( target_address + 1 ) - page_start,
                PROT_READ | PROT_EXEC
            ) == 0
          )
        {
            return true;
        }

        __LOG(
            "Failed to reset page permissions at %llx",
            ( long long unsigned int ) target_address
        );
    }

    return true;
}
