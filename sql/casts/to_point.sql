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
