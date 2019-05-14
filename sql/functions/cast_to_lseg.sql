CREATE OR REPLACE FUNCTION __cast_polygon_to_lseg( in_polygon POLYGON )
RETURNS LSEG AS
 'pgpolybool.so', '__cast_polygon_to_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_point_to_lseg( in_point POINT )
RETURNS LSEG AS
 'pgpolybool.so', '__cast_point_to_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_line_to_lseg( in_line LINE )
RETURNS LSEG AS
 'pgpolybool.so', '__cast_line_to_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_path_to_lseg( in_path PATH )
RETURNS LSEG AS
 'pgpolybool.so', '__cast_path_to_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_circle_to_lseg( in_circle CIRCLE )
RETURNS LSEG AS
 'pgpolybool.so', '__cast_circle_to_lseg'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
