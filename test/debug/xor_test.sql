SELECT fn_xor_polygons( '((0,0),(5,0),(5,5),(0,5))'::POLYGON, '((2.5,0),(5,0),(5,5),(2.5,5))'::POLYGON );

-- Should return ((0,0),(2.5,0),(2.5,5),(0,5))
