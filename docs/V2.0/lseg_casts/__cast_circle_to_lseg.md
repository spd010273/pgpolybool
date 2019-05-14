## __cast_circle_to_lseg

### Prototype:

LSEG __cast_circle_to_lseg( CIRCLE )

### Description:

Casts the input CIRCLE to a line segment by generating a segment that bisects the circle. The direction of the segment is from the origin to the center of the circle, and the length is the diameter.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Emits an error and aborts the transaction when the quadratic solution is degenerate.
