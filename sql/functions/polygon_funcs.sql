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

CREATE OR REPLACE FUNCTION fn_get_points_ombb( in_poly POLYGON )
RETURNS POINT[] AS
 'pgpolybool.so', 'fn_get_points_ombb'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_polygon_ombb( in_poly POLYGON )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_get_polygon_ombb'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_ombb( in_poly POLYGON )
RETURNS TABLE
(
    bounding_box    POLYGON,
    width           DOUBLE PRECISION,
    height          DOUBLE PRECISION,
    angle           DOUBLE PRECISION
) AS
 'pgpolybool.so', 'fn_get_ombb'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_convex_hull( in_poly POLYGON )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_get_convex_hull_polygon'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_convex_hull( in_point_array POINT[] )
RETURNS POLYGON AS
 'pgpolybool.so', 'fn_get_convex_hull_point_array'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
