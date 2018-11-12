SELECT fn_subtract_polygons( '((0,0),(5,0),(5,5),(0,5))'::POLYGON, '((0,0),(2.5,0),(2.5,5),(0,5))'::POLYGON );

-- Should return ((2.5,0),(5,0),(5,5),(2.5,5))
