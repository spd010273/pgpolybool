## __cast_point_to_line

### Prototype:

LINE __cast_point_to_line( POINT )

### Description:

Casts the input POINT to a line by finding a line that passes through the POINT and the origin. Behavior is undefined for Points that are at the origin.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
