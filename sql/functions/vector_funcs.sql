/* Vector mathematical functions */
CREATE OR REPLACE FUNCTION fn_cross_product( POINT, POINT )
RETURNS DOUBLE PRECISION AS
 'pgpolybool.so', 'fn_cross_product'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Vector conversion functions */
CREATE OR REPLACE FUNCTION fn_lseg_to_vector( LSEG )
RETURNS POINT AS
 'pgpolybool.so', 'fn_lseg_to_vector'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
