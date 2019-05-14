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
