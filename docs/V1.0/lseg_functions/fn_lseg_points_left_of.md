## fn_lseg_points_left_of

### Prototype:

BOOLEAN fn_lseg_points_left_of( LSEG, LSEG )

### Description:

Returns whether a line segment points to the left of the first LSEG argument. This is determined by the cross product of the vector conversion of the two inputs.

### Error conditions:

Emits an error and aborts the transaction on failure to allocate memory.
