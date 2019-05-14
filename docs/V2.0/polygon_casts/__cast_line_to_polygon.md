## __cast_line_to_polygon

### Prototype:

POLYGON __cast_line_to_polygon( LINE )

### Description:

Cast the input LINE to a polygon by generating a square of legnth / width 1 about the y-intercept of the line.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
