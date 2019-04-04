CREATE OR REPLACE FUNCTION __cast_line_to_point( in_line LINE )
RETURNS POINT AS
 'pgpolybool.so', '__cast_line_to_point'
LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE OR REPLACE FUNCTION __cast_path_to_point( in_path PATH )
RETURNS POINT AS
 'pgpolybool.so', '__cast_path_to_point'
LANGUAGE C IMMUTABLE PARALLEL SAFE;
