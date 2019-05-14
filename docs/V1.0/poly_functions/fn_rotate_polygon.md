## fn_rotate_polygon

### Prototype

POLYGON fn_rotate_polygon( POLYGON, DOUBLE PRECISION )

### Description

Rotates a polygon about its centroid by DOUBLE PRECISION radians.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Emits an error and aborts the transaction on out-of-bound radian arguments (-2 pi to 2 pi, exclusive)
