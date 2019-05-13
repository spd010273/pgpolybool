# fn_scale_lseg

## Prototype:

LSEG fn_scale_lseg( LSEG, DOUBLE PRECISION, POINT )

## Description:

Scales the input line segment LSEG by the DOUBLE PRECISION factor about the specified reference POINT. The reference is optional and assumed to be the midpoint of the segment.

## Error Conditions:

Emits an error and returns NULL if the quadratic solution is degenerate. Emits an error and aborts the transaction on failure to allocate memory.
