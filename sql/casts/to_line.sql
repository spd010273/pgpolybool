CREATE CAST ( POLYGON AS LINE )
    WITH FUNCTION __cast_polygon_to_line( POLYGON )
    AS IMPLICIT;

CREATE CAST ( POINT AS LINE )
    WITH FUNCTION __cast_point_to_line( POINT )
    AS IMPLICIT;

CREATE CAST ( LSEG AS LINE )
    WITH FUNCTION __cast_lseg_to_line( LSEG )
    AS IMPLICIT;

CREATE CAST ( PATH AS LINE )
    WITH FUNCTION __cast_path_to_line( PATH )
    AS IMPLICIT;

CREATE CAST ( BOX AS LINE )
    WITH FUNCTION __cast_box_to_line( BOX )
    AS IMPLICIT;

CREATE CAST ( CIRCLE AS LINE )
    WITH FUNCTION __cast_circle_to_line( CIRCLE )
    AS IMPLICIT;

