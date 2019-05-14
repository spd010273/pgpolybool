# fn_subtract_polygons

## Prototype:

POLYGON fn_subtract_polygons( POLYGON[] )
POLYGON fn_subtract_polygons( POLYGON, POLYGON )

## Description

Compute the difference between two or more polygons.

In the current implementation, in cases where the Martinez algorithm returns multiple polygons, this function selects the polygon with the largest enclosed area as the result. Due to limitations in PostgreSQL polygon storage, any resulting holes cannot be expressed and are discarded.

## Error Conditions:

This function will emit an error and abort the transaction if there is a failure to allocate memory.
