-- Attempt to overwrite the function address of path_center
CREATE OR REPLACE FUNCTION __overload_path_center()
RETURNS VOID AS
 'pgpolybool.so', '__overload_path_center'
LANGUAGE C IMMUTABLE PARALLEL UNSAFE;
