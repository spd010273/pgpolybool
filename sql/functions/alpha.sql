CREATE OR REPLACE FUNCTION fn_get_alpha_shape( in_points POINT[], in_alpha DOUBLE PRECISION )
RETURNS POLYGON[] AS
 'pgpolybool.so', 'fn_get_alpha_shape'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
