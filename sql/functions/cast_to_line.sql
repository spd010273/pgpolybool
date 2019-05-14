CREATE OR REPLACE FUNCTION __cast_polygon_to_line( in_polygon POLYGON )
RETURNS LINE AS
 'pgpolybool.so', '__cast_polygon_to_line'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_point_to_line( in_point POINT )
RETURNS LINE AS
 'pgpolybool.so', '__cast_point_to_line'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_lseg_to_line( in_lseg LSEG )
RETURNS LINE AS
 'pgpolybool.so', '__cast_lseg_to_line'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_path_to_line( in_path PATH )
RETURNS LINE AS
 'pgpolybool.so', '__cast_path_to_line'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_box_to_line( in_box BOX )
RETURNS LINE AS
 'pgpolybool.so', '__cast_box_to_line'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_circle_to_line( in_circle CIRCLE )
RETURNS LINE AS
 'pgpolybool.so', '__cast_circle_to_line'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
