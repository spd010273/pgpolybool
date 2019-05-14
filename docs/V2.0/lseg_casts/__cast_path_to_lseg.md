## __cast_path_to_lseg

### Prototype:

LSEG __cast_path_to_lseg( PATH )

### Description:

Casts the input PATH by finding the best-fit line for the paths points. The line segment is contained within the axis-oriented bounding box of the path.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory. Returns NULL on failure to compute the best-fit line for the path.
