/* Line Segment geometric functions */
CREATE OR REPLACE FUNCTION fn_get_polygon_line_segment_distance( poly POLYGON, seg LSEG )
RETURNS DOUBLE PRECISION AS
 'pgpolybool.so', 'fn_get_polygon_lseg_distance'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Get angle between two line segments */
CREATE OR REPLACE FUNCTION fn_get_lseg_angle( line LSEG, reference LSEG )
RETURNS DOUBLE PRECISION AS
 'pgpolybool.so', 'fn_get_lseg_angle'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Returns the line segment that is furthest from away_point */
CREATE OR REPLACE FUNCTION fn_get_parallel_segment( seg LSEG, away_point POINT DEFAULT NULL, distance DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG AS
 'pgpolybool.so', 'fn_get_parallel_segment'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Returns two segments that are parallel to the inout and the specified distance away */
CREATE OR REPLACE FUNCTION fn_get_parallel_segments( seg LSEG, distance DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG[] AS
 'pgpolybool.so', 'fn_get_parallel_segments'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Returns the orthoganol segment furthest from away_point */
CREATE OR REPLACE FUNCTION fn_get_orthogonal_segment( seg LSEG, away_point POINT DEFAULT NULL, length DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG AS
 'pgpolybool.so', 'fn_get_orthogonal_segment'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Returns two segments that are orthogonal to the input and of specified length */
CREATE OR REPLACE FUNCTION fn_get_orthogonal_segments( seg LSEG, length DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG[] AS
 'pgpolybool.so', 'fn_get_orthogonal_segments'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Scales a line segment about reference point (which must lie on the segment) by the percentage scale_factor */
CREATE OR REPLACE FUNCTION fn_scale_lseg( seg LSEG, scale_factor DOUBLE PRECISION DEFAULT 1.0, reference_point POINT DEFAULT NULL )
RETURNS LSEG AS
 'pgpolybool.so', 'fn_scale_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_lseg_points_right_of( segment LSEG, reference LSEG )
RETURNS BOOLEAN AS
 'pgpolybool.so', 'fn_lseg_points_right_of'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_lseg_points_left_of( segment LSEG, reference LSEG )
RETURNS BOOLEAN AS
 'pgpolybool.so', 'fn_lseg_points_left_of'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
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
/* Polygon decomposition functions */

/* Rotate polygon about its center */
CREATE OR REPLACE FUNCTION fn_get_polygon_points( poly POLYGON )
RETURNS POINT[] AS
 'pgpolybool.so', 'fn_get_polygon_points'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_polygon_line_segments( poly POLYGON )
RETURNS LSEG[] AS
 'pgpolybool.so', 'fn_get_polygon_line_segs'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Polygon geometric functions */
CREATE OR REPLACE FUNCTION fn_rotate_polygon( poly POLYGON, radians DOUBLE PRECISION )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_rotate_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_polygon_area( poly POLYGON )
RETURNS DOUBLE PRECISION AS
 'pgpolybool.so', 'fn_get_polygon_area'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Polygon conversion functions */
CREATE OR REPLACE FUNCTION fn_box_to_polygon( BOX )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_box_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_points_to_polygon_array( points_array POINT[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_points_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_points_to_polygon( VARIADIC points_array POINT[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_points_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_lseg_to_polygon( seg LSEG, width DOUBLE PRECISION DEFAULT 1.0 )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_lseg_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
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
