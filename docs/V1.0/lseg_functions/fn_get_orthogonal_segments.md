# fn_get_orthogonal_segments

## Prototype:

LSEG[] fn_get_orthogonal_segments( LSEG, DOUBLE PRECISION )

## Description:

Returns the two line segments that are orthogonal to the input. These segments begin at the inputs midpoint and are of length DOUBLE PRECISION.

## Error Conditions:

Emits and error and returns NULL in cases where the quadratic solution is degenerate. Emits an error and aborts the transaction on failure to allocate memory.
