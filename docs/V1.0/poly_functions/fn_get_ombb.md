## fn_get_ombb

### Prototype

TABLE ( fn_get_ombb( POLYGON ) )
(
    bounding_box POLYGON,
    width        DOUBLE PRECISION,
    height       DOUBLE PRECISION,
    angle        DOUBLE PRECISION
)

### Description:

Record-returning function that returns detailed information about the oriented minimum bounding box of the input POLYGON. This inclused the bounding box itself, its width, height, and angle.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
