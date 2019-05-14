## fn_get_orthogonal_segment

### Prototype:

LSEG fn_get_orthogonal_segment( LSEG, POINT, DOUBLE PRECISION )

### Description:

Returns the line segment that is orthogonal to the input line. The segment originates at the inputs midpoint and radiates in the direction opposite of the specified POINT. The output will be of length DOUBLE PRECISION.

### Error Conditions:

Emits an error and returns NULL in cases where the quadratic solution is degenerate. Emits and error and aborts the transaction on failure to allocate memory.
