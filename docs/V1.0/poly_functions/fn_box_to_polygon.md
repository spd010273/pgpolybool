## fn_box_to_polygon

### Prototype:

POLYGON fn_box_to_polygon( BOX )

### Description:

Converts the input BOX to a POLYGON.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Emits an error and returns NULL when quadratic solution is degenerate.
