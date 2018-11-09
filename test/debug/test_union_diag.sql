DROP EXTENSION IF EXISTS pgpolybool;
CREATE EXTENSION pgpolybool;
SET client_min_messages = DEBUG;
SELECT pg_backend_pid();
SELECT fn_union_polygons(
    ARRAY[
        '((10,0),(20,10),(10,20),(0,10))',
        '((20,10),(30,20),(20,30),(10,20))'
    ]::POLYGON[]
);
