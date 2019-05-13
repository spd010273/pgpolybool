# fn_get_parallel_segment

## Prototype:

LSEG fn_get_parallel_segment( LSEG, POINT, DOUBLE PRECISION )

## Description:

Returns the line segment parallel to the input that is furthest away from the specified POINT, and is of distance DOUBLE PRECISION away from the input line segment.

## Error Conditions:

Emits an error and returns NULL in cases where the quadratic solution is degenerate. Emits an error and aborts the transaction on failure to allocate memory.
