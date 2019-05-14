## __cast_circle_to_line

### Prototype:

LINE __cast_circle_to_line( CIRCLE )

### Description:

A rather dubious function but included for completeness. This function partially encodes information about the circle into the line equation. The slope of the line is the radius of the circle, and the y-intercept becomes the y-coordinate of the circles center.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
