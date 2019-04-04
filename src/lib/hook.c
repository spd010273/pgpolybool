#include "hook.h"

// Simple hooking for x86_64 in Linux

bool __hook( uintptr_t target_address, uintptr_t hooking_function )
{
    uint64_t  instruction_one = 0;
    uint64_t  instruction_two = 0;
    uintptr_t page_start      = 0;
    size_t    page_size       = 0;

    __LOG(
        "Attempting to hook\n 0x%llx\n with\n 0x%llx\n",
        ( long long unsigned int ) target_address,
        ( long long unsigned int ) hooking_function
    );

    if( target_address == 0 || hooking_function == 0 )
    {
        return false;
    }

    page_size  = sysconf( _SC_PAGESIZE );
    page_start = target_address & -page_size;

    if(
        mprotect(
            ( void * ) page_start,
            ( target_address + 1 ) - page_start,
            PROT_READ | PROT_WRITE | PROT_EXEC
        ) == 0
      )
    {
        /*
         *  Overwrite &target_address with the following
         *
         *   push &hooking_function[0..31]
         *   mov [rsp + 4] &hooking_function[32..63]
         *   ret
         *
         *  Which translates to the following machine code
         *   0x<address_0_31>68
         *   0x<address_32_63>042444C7
         *   0xC3
         *
         *  This is packed and aligned to 64 bits, this ends up spreading the
         *  mov [rsp+4] across two QWORDS.
         */
        // TODO: It seems we aren't getting the appropriate address for the hooking function?
        instruction_one = 0x2444C70000000068 | ( ( ( uint32_t ) hooking_function ) << 8 );
        instruction_two = 0x0000C30000000004 | ( ( ( uint32_t ) ( hooking_function >> 32 ) ) << 8 );
        *( ( uintptr_t * )( target_address     ) ) = ( uintptr_t ) instruction_one;
        *( ( uintptr_t * )( target_address + 8 ) ) = ( uintptr_t ) instruction_two;

        __LOG(
            "Wrote the following instructions to:\n 0x%llx: 0x%llx\n 0x%llx: 0x%llx\n"\
            "Actual instructions are\n 0x%llx\n 0x%llx\n",
            ( long long unsigned int ) ( uintptr_t ) target_address,
            ( long long unsigned int ) *( ( uintptr_t * ) target_address ),
            ( long long unsigned int ) ( uintptr_t ) ( target_address + 8 ),
            ( long long unsigned int ) *( ( uintptr_t * ) (target_address + 8 ) ),
            ( long long unsigned int ) instruction_one,
            ( long long unsigned int ) instruction_two
        );
    }
    else
    {
        return false;
    }

    return true;

}

uintptr_t __get_foreign_function_address( char * function_name )
{
    void *     program_handle   = NULL;
    uint64_t * function_address = NULL;

    if( function_name == NULL )
    {
        return 0;
    }

    program_handle = dlopen( NULL, RTLD_NOW );

    if( program_handle == NULL )
    {
        __LOG(
            "Failed to open program symbol table: %p",
            program_handle
        );

        return 0;
    }

    function_address = dlsym( program_handle, function_name );

    if( function_address == NULL )
    {
        __LOG(
            "Could not locate '%s' in symbol table %p",
            function_name,
            program_handle
        );

        return 0;
    }

    return ( uintptr_t ) function_address;
}

bool hook_function( char * foreign_function, uintptr_t override_function )
{
    uintptr_t target_function = 0;

    if( foreign_function == NULL )
    {
        return false;
    }

    if( override_function == 0 )
    {
        // Why are you trying to make us jump to 0x0?
        return false;
    }

    target_function = __get_foreign_function_address( foreign_function );

    if( target_function == 0 )
    {
        return false;
    }

    return __hook( target_function, override_function );
}
