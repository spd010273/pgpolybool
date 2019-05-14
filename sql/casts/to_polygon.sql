/*
 * Generates a BOX as a POLYGON using the LSEG as a diagonal of the BOX. The
 * resulting polygon will be four points.
 */

CREATE CAST (LSEG AS POLYGON)
    WITH FUNCTION __cast_lseg_to_polygon( LSEG )
    AS IMPLICIT;

/*
 * Similar to LSEG conversion, the LINE will pass through the diagonal of the
 * BOX used to form the simple polygon. The center of the box will be at the
 * y-intercept of the line and the length of the diagonal is sqrt(2), making
 * the sides have length 1
 */
CREATE CAST (LINE AS POLYGON)
    WITH FUNCTION __cast_line_to_polygon( LINE )
    AS IMPLICIT;

/*
 * Generates a polygon circle with twelve points and diameter 1 about the point
 */
CREATE CAST (Point AS POLYGON)
    WITH FUNCTION __cast_point_to_polygon( POINT )
    AS IMPLICIT;

/*
CREATE CAST (VECTOR AS POLYGON)
    WITH FUNCTION fn_vector_to_polygon( VECTOR )
    AS IMPLICIT; 
*/
