# fn_get_polygon_line_segment_distance

## Prototype:

DOUBLE PRECISION fn_get_polygon_line_segment_distance( POLYGON, LSEG )

## Description:

Return the shortest-path distance between the specified polygon and line segment.

## Error Conditions:

This function will emit an error and abort the transaction if there is a failure to allocate memory. The function will return NULL on NULL input, or in cases where the input polygon has 1 or fewer points.
