DROP EXTENSION pgpolybool;
CREATE EXTENSION pgpolybool;
SET client_min_messages = DEBUG;
SELECT fn_union_polygons( ARRAY[ '((0,0),(5,0),(5,5),(0,5))','((5,0),(10,0),(10,5),(5,5))' ]::POLYGON[] );
SELECT pg_backend_pid();
