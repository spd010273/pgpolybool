# fn_get_parallel_segments

## Prototype:

LSEG[] fn_get_parallel_segments( LSEG, DOUBLE PRECISION )

## Description:

Returns the two line segments parallel to the input that are distance DOUBLE PRECISION away from the input.

## Error Conditions:

Emits and error and returns NULL in cases where the quadratic solution is degenerate. Emits an error and aborts the transaction on failure to allocate memory.
