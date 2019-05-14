## __cast_path_to_point

### Prototype:

POINT __cast_path_to_point( PATH )

### Description:

Casts the input PATH to a point by determining the centroid of the path.

### Notes:

This function requires pgpolybool to either be:

* Added to PostgreSQLs `shared_preload_libraries`
* Call __overload_path_center SQL function

These will hook the function path_center ( src/backend/utils/adt/geo_ops.c ) stub in memory.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. If the function has not been hooked, path_center will return an implementation warning.
