## fn_get_root_orthogonal_segment

### Prototype:

LSEG fn_get_root_orthogonal_segment( LSEG, POINT, DOUBLE PRECISION )

### Description:

Returns a segment of length DOUBLE PRECISION that is orthogonal to the input LSEG at the specified endpoint (POINT). The POINT argument must be an endpoint to the input LSEG. The resulting segment will have a midpoint that is the specified endpoint.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Emits an error and returns NULL when the quadratic solution is degenerate.
