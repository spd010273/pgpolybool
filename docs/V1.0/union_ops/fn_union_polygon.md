## fn_union_polygons

### Prototype:

POLYGON fn_union_polygons( POLYGON[] )
POLYGON fn_union_polygons( POLYGON, POLYGON )

### Description:

Union two or more polygons into a single polygon.

This function scales the input polygons by a constant ZOOM_RATE (defined in src/lib/polyprocessing.h). In cases where the greater than two polygons are provided as input, the polygon array is ordered by center-to-center distance from an arbitrary array member prior to iterative calling of the UNION function. Due to the nature of PostgreSQLs storage of polygons, only a single polygon may be returned. In cases where the Martinez algorithms generates multiple output polygons, the polygon with the largest enclosed area is selected as the output. While the Martinez algorithm is capable of computing the holes resulting in the union of two polygons, this cannot be expressed in the result of this function. Prior to returning, the result polygon is scaled back down by 1 - ZOOM_RATE.

### Error Conditions:

This function will emit an error and abort the transaction if there is a failure to allocate memory. The function will return NULL if there is no intersection between the polygons.
