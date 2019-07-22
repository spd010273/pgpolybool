DO
 $_$
DECLARE
    my_function_list VARCHAR[] := ARRAY[
        'public.fn_intersect_polygons( POLYGON[] )',
        'public.fn_intersect_polygons( POLYGON, POLYGON )',
        'public.fn_subtract_polygons( POLYGON[] )',
        'public.fn_subtract_polygons( POLYGON, POLYGON )',
        'public.fn_union_polygons( POLYGON[] )',
        'public.fn_union_polygons( POLYGON, POLYGON )',
        'public.fn_xor_polygons( POLYGON[] )',
        'public.fn_xor_polygons( POLYGON, POLYGON )',
        'public.fn_get_lseg_angle( LSEG, LSEG )',
        'public.fn_get_orthogonal_segment( LSEG, POINT, DOUBLE PRECISION )',
        'public.fn_get_orthogonal_segments( LSEG, DOUBLE PRECISION )',
        'public.fn_get_parallel_segment( LSEG, POINT, DOUBLE PRECISION )',
        'public.fn_get_parallel_segments( LSEG, DOUBLE PRECISION )',
        'public.fn_get_polygon_line_segment_distance( POLYGON, LSEG )',
        'public.fn_get_root_orthogonal_segment( LSEG, POINT, DOUBLE PRECISION )',
        'public.fn_lseg_points_left_of( LSEG, LSEG )',
        'public.fn_lseg_points_right_of( LSEG, LSEG )',
        'public.fn_point_is_right_of_lseg( LSEG, POINT, POINT )',
        'public.fn_point_is_left_of_lseg( LSEG, POINT, POINT )',
        'public.fn_scale_lseg( LSEG, DOUBLE PRECISION, POINT )',
        'public.fn_box_to_polygon( BOX )',
        'public.fn_get_ombb( POLYGON )',
        'public.fn_get_points_ombb( POLYGON )',
        'public.fn_get_polygon_area( POLYGON )',
        'public.fn_get_polygon_line_segments( POLYGON )',
        'public.fn_get_polygon_ombb( POLYGON )',
        'public.fn_get_polygon_points( POLYGON )',
        'public.fn_lseg_to_polygon( LSEG, DOUBLE PRECISION )',
        'public.fn_points_to_polygon( VARIADIC POINT[] )',
        'public.fn_rotate_polygon( POLYGON, DOUBLE PRECISION )'
    ]::VARCHAR[];
    my_function VARCHAR := '';
BEGIN
    FOR my_function IN( SELECT unnest( my_function_list ) AS function ) LOOP
        EXECUTE 'GRANT EXECUTE ON FUNCTION ' || my_function || ' TO public';
    END LOOP;
END
 $_$
    LANGUAGE 'plpgsql';
