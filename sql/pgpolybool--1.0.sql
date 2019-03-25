/* PGPolyBool functions */
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


/* Helper functions */
CREATE OR REPLACE FUNCTION fn_rotate_polygon( poly POLYGON, radians DOUBLE PRECISION )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_rotate_polygon';

CREATE OR REPLACE FUNCTION fn_get_polygon_points( poly POLYGON )
RETURNS POINT[] IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_points';

CREATE OR REPLACE FUNCTION fn_get_polygon_line_segments( poly POLYGON )
RETURNS LSEG[] IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_line_segs';

CREATE OR REPLACE FUNCTION fn_get_polygon_area( poly POLYGON )
RETURNS DOUBLE PRECISION IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_area';

CREATE OR REPLACE FUNCTION fn_line_segments_intersect( a LSEG, b LSEG )
RETURNS BOOLEAN IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_lseg_intersect';

CREATE OR REPLACE FUNCTION fn_line_segments_intersection_point( a LSEG, b LSEG )
RETURNS POINT IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_lseg_intersect_point';

CREATE OR REPLACE FUNCTION fn_get_polygon_line_segment_distance( poly POLYGON, seg LSEG )
RETURNS DOUBLE PRECISION IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_polygon_lseg_distance';

CREATE OR REPLACE FUNCTION fn_get_parallel_segment( seg LSEG, away_point POINT DEFAULT NULL, distance DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_parallel_segment';

CREATE OR REPLACE FUNCTION fn_get_parallel_segments( seg LSEG, distance DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG[] IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_parallel_segments';

CREATE OR REPLACE FUNCTION fn_get_orthogonal_segment( seg LSEG, away_point POINT DEFAULT NULL, length DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_orthogonal_segment';

CREATE OR REPLACE FUNCTION fn_get_orthogonal_segments( seg LSEG, length DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG[] IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_orthogonal_segments';

CREATE OR REPLACE FUNCTION fn_lseg_to_polygon( seg LSEG, width DOUBLE PRECISION DEFAULT 1.0 )
RETURNS POLYGON IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_lseg_to_polygon';

CREATE OR REPLACE FUNCTION fn_scale_lseg( seg LSEG, scale_factor DOUBLE PRECISION DEFAULT 1.0, reference_point POINT DEFAULT NULL )
RETURNS LSEG IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_scale_lseg';

CREATE OR REPLACE FUNCTION fn_create_reflected_box( line LSEG, ortho LSEG )
RETURNS BOX IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_create_reflected_box';

CREATE OR REPLACE FUNCTION fn_get_lseg_angle( line LSEG, reference LSEG )
RETURNS BOX IMMUTABLE PARALLEL SAFE LANGUAGE C AS 'pgpolybool.so', 'fn_get_lseg_angle';
