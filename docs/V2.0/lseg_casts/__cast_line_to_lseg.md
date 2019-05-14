## __cast_line_to_lseg

### Prototype:

LSEG __cast_line_to_lseg( LINE )

### Description:

Casts the input LINE to a line segment by returning a segment of the line of unit length which passes through an axis. If the line passes through both the y and x axis, the result segments midpoint will be encoded as both the x and y intercepts.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
