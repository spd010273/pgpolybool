# fn_intersect_polygons

## Prototype:

POLYGON fn_intersect_polygons( POLYGON[] )
POLYGON fn_intersect_polygons( POLYGON, POLYGON )

## Description:

Find the intersection between two or more polygons.

This function returns the largest intersection area of the provided POLYGONs. In cases where greater than two polygons are provided as input, the polygon array is ordered by the center-to-center distance from an arbitrary array member prior to iterative calling of the INTERSECT function.

## Error Conditions:

This function will emit an error and abort the transaction if there is a failure to allocate memory. The function will return NULL if there is no intersection between the polygons.
