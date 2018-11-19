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

CREATE OR REPLACE FUNCTION fn_rotate_polygon( poly POLYGON, radians DOUBLE PRECISION )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_rotate_polygon';
CREATE OR REPLACE FUNCTION fn_get_polygon_points( poly POLYGON )
RETURNS POINT[] IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_points';
CREATE OR REPLACE FUNCTION fn_get_polygon_line_segments( poly POLYGON )
RETURNS LSEG[] IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_line_segs';
CREATE OR REPLACE FUNCTION fn_line_segments_intersect( a LSEG, b LSEG )
RETURNS BOOLEAN IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_lseg_intersect';
CREATE OR REPLACE FUNCTION fn_line_segments_intersection_point( a LSEG, b LSEG )
RETURNS POINT IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_lseg_intersect_point';
CREATE OR REPLACE FUNCTION fn_line_segments_distance( a LSEG, b LSEG )
RETURNS DOUBLE PRECISION IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_lseg_distance';
CREATE OR REPLACE FUNCTION fn_get_polygon_line_segment_distance( poly POLYGON, seg LSEG )
RETURNS DOUBLE PRECISION IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_lseg_distance';
CREATE OR REPLACE FUNCTION fn_get_orthogonal_segment( seg LSEG, away_point POINT DEFAULT NULL, length DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_orthogonal_segment';
