## __cast_polygon_to_lseg

### Prototype:

LSEG __cast_polygon_to_lseg( POLYGON )

### Description:

Casts the input POLYGON to a line segment by computing the best-fit line for the polygons points. The segment is fully contained by the polygons axis-oriented bounding box.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Returns NULL on failure to compute the best-fit line.
