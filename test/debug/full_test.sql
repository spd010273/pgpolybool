SET client_min_messages = DEBUG;
SELECT pg_backend_pid();
    SELECT COUNT( lms.location_map_space ),
           array_agg( fn_rotate_polygon( lms.perimeter, radians( lms.rotation ) ) ),
           fn_union_polygons( array_agg( fn_rotate_polygon( lms.perimeter, radians( lms.rotation ) ) ) )
      FROM tb_location_map_space lms
      JOIN tb_logical_Space_alignment lsa
        ON lsa.physical_space = lms.space
  GROUP BY lms.location_map,
           lsa.logical_space;
