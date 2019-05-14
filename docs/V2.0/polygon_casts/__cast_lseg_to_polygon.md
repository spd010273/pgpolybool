## __cast_lseg_to_polygon

### Prototype

POLYGON __cast_lseg_to_polygon( LSEG )

### Description

Casts the input LSEG to a polygon by forming a rectangle around the LSEG, where the lseg forms the diagonal of the rectangle.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Emits an error and aborts the transaction when the solution to the quadratic function is degenerate.
