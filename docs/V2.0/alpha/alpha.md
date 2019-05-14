# Alpha Shape

Note: This function is incomplete

## fn_get_alpha_shape

### Prototype:

POLYGON[] fn_get_alpha_shape( POINT[], DOUBLE PRECISION )

### Description:

Computes the alpha shape for the input POINT[] field. The DOUBLE PRECISION parameter is the alpha parameter for the function.

### Error Conditions

Emits an error and aborts the transaction on failure to allocate memory.
