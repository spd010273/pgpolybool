CREATE OR REPLACE FUNCTION fn_intersect_polygons( poly_array POLYGON[] )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_intersect_polygons_array';

CREATE OR REPLACE FUNCTION fn_intersect_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_intersect_polygons';

CREATE OR REPLACE FUNCTION fn_subtract_polygons( poly_array POLYGON[] )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_subtract_polygons_array';

CREATE OR REPLACE FUNCTION fn_subtract_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_subtract_polygons';

CREATE OR REPLACE FUNCTION fn_union_polygons( poly_array POLYGON[] )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_union_polygons_array';

CREATE OR REPLACE FUNCTION fn_union_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_union_polygons';

CREATE OR REPLACE FUNCTION fn_xor_polygons( poly_array POLYGON[] )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_xor_polygons_array';

CREATE OR REPLACE FUNCTION fn_xor_polygons( poly_a POLYGON, poly_b POLYGON )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_xor_polygons';
--TODO: Add aggregate functions
