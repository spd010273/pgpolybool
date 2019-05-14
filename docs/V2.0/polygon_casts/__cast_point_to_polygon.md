## __cast_point_to_polygon

### Prototype:

POLYGON __cast_point_to_polygon( POINT )

### Description:

Casts the input POINT to a polygon by generating a 12-sided circle about the point, with radius 1.0

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
