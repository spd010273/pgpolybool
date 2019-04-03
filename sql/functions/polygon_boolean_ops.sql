/* Intersection */
CREATE OR REPLACE FUNCTION fn_intersect_polygons( poly_array POLYGON[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_intersect_polygons_array'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_intersect_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_intersect_polygons'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Subtraction */
CREATE OR REPLACE FUNCTION fn_subtract_polygons( poly_array POLYGON[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_subtract_polygons_array'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_subtract_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_subtract_polygons'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Union */
CREATE OR REPLACE FUNCTION fn_union_polygons( poly_array POLYGON[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_union_polygons_array'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_union_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_union_polygons'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Exclusive OR */
CREATE OR REPLACE FUNCTION fn_xor_polygons( poly_array POLYGON[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_xor_polygons_array'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_xor_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_xor_polygons'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
