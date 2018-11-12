SELECT fn_intersect_polygons( '((0,0),(5,0),(5,5),(0,5))'::POLYGON, '((4,0),(6,0),(6,5),(4,5))'::POLYGON );

--should return ((4,0),(5,0),(5,5),(4,5))
