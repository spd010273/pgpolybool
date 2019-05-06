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

CREATE OR REPLACE FUNCTION fn_extend_lseg( seg LSEG, endpoint POINT, length DOUBLE PRECISION DEFAULT 1.0 )
RETURNS LSEG AS
 'pgpolybool.so', 'fn_extend_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_lseg_points_right_of( segment LSEG, reference LSEG )
RETURNS BOOLEAN AS
 'pgpolybool.so', 'fn_lseg_points_right_of'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_lseg_points_left_of( segment LSEG, reference LSEG )
RETURNS BOOLEAN AS
 'pgpolybool.so', 'fn_lseg_points_left_of'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION fn_get_root_orthogonal_segment( segment LSEG, endpoint POINT, length DOUBLE PRECISION )
RETURNS LSEG AS
 'pgpolybool.so', 'fn_get_root_orthogonal_segment'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
