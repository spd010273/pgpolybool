## fn_lseg_to_polygon

### Prototype:

POLYGON fn_lseg_to_polygon( LSEG, DOUBLE PRECISION )

### Description:

Converts a line segment LSEG to a POLYGON. The LSEG is expanded from a 1-dimension object to a 2-dimension object of width DOUBLE PRECISION

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
