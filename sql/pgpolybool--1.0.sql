CREATE OR REPLACE FUNCTION __cast_line_to_point( in_line LINE )
RETURNS POINT AS
 'pgpolybool.so', '__cast_line_to_point'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_path_to_point( in_path PATH )
RETURNS POINT AS
 'pgpolybool.so', '__cast_path_to_point'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION __cast_lseg_to_polygon( in_segment LSEG )
RETURNS POLYGON AS
 'pgpolybool.so', '__cast_lseg_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_line_to_polygon( in_line LINE )
RETURNS POLYGON AS
 'pgpolybool.so', '__cast_line_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_point_to_polygon( in_point POINT )
RETURNS POLYGON AS
 'pgpolybool.so', '__cast_point_to_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Attempt to overwrite the function address of path_center
CREATE OR REPLACE FUNCTION __overload_path_center()
RETURNS VOID AS
 'pgpolybool.so', '__overload_path_center'
LANGUAGE C IMMUTABLE PARALLEL UNSAFE;
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
/* Returns the y-intercept of the line */
CREATE CAST ( LINE AS POINT )
    WITH FUNCTION __cast_line_to_point( LINE )
    AS IMPLICIT;

/*
 *  TODO: This function is defined and has an entry point in src/backend/utils/adt/geo_ops.c
 *  but is just a stub that emits an error and returns NULL. Because the CREATE CAST functionality
 *  does not support create or replace syntax styles, we cannot override the existing dummy cast
 *  which leaves the only way to extend this cast to actually work on the path->point transition
 *  would be to overwrite the address of Datum path_center( PG_FUNCTION_ARGS ) with our own function
 *  which is a little dangerous and hacky :(
-- Returns the centroid of the path
CREATE CAST ( PATH AS POINT )
    WITH FUNCTION __cast_path_to_point( PATH )
    AS IMPLICIT;
*/
/*
 * Generates a BOX as a POLYGON using the LSEG as a diagonal of the BOX. The
 * resulting polygon will be four points.
 */

CREATE CAST (LSEG AS POLYGON)
    WITH FUNCTION __cast_lseg_to_polygon( LSEG )
    AS IMPLICIT;

/*
 * Similar to LSEG conversion, the LINE will pass through the diagonal of the
 * BOX used to form the simple polygon. The center of the box will be at the
 * y-intercept of the line and the length of the diagonal is sqrt(2), making
 * the sides have length 1
 */
CREATE CAST (LINE AS POLYGON)
    WITH FUNCTION __cast_line_to_polygon( LINE )
    AS IMPLICIT;

/*
 * Generates a polygon circle with twelve points and diameter 1 about the point
 */
CREATE CAST (Point AS POLYGON)
    WITH FUNCTION __cast_point_to_polygon( POINT )
    AS IMPLICIT;

/*
CREATE CAST (VECTOR AS POLYGON)
    WITH FUNCTION fn_vector_to_polygon( VECTOR )
    AS IMPLICIT; 
*/
