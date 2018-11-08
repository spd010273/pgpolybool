pgpolybool
==========

# Summary

Polygon arithmatic operations for native PostgreSQL POLYGON type.

pgpolybool provides the following functions:

* POLYGON fn_union_polygons( POLYGON[] ): UNION 2..n polygons.
* POLYGON fn_union_polygons( POLYGON, POLYGON ): UNION two polygons.
* POLYGON fn_intersect_polygons( POLYGON[] ): INTERSECT 2..n polygons.
* POLYGON fn_intersect_polygons( POLYGON, POLYGON ): INTERSECT two polygons.
* POLYGON fn_subtract_polygons( POLYGON[] ): Subtract 2..n polygons.
* POLYGON fn_subtract_polygons( POLYGON, POLYGON ): Subtract two polygons.
* POLYGON fn_xor_polygons( POLYGON[] ): XOR 2..n polygons.
* POLYGON fn_xor_polygons( POLYGON, POLYGON ): XOR two polygons.

pgpolybool implements the clipping algorithm developed in Martinez, Rueda, Feito 2009 paper published the cageo journal (see /docs/martinez_boolean.pdf).

# Usage

# Installation

# Special Thanks

Francisco Martinez, Antonio Jesus Reuda, and Francisco Ramon Feito for deveoping this algorithm, it is extremely versatile and fast. It excelled in cases where both the Vatti algorithm or Greneir-Horrman failed or could not be modified to handle colinearity edge cases.

Sean Connelly for his work at (http://sean.cm/a/polygon-clipping-pt2). This post contains a very well illustrated and articulated walkthrough of the algorithm's operation. There is also a interactive tool for playing around with polygon inputs to the algorithm.

PostgreSQL Global Development Group, for their finely maintained project, exceptional documentation, and enthusiastic user base.

Richard Davies for encouragments, jokes, insights and tricks in C/C++.

My employer, Nead Werx, Inc. for granting me the time to bring this implementation to PostgreSQL. I hope later to get this into the PostGIS codebase ;)

# Coming Soon

I seek to add the following features in future releases:
* Output filter to remove redundant colinear points in output
* Aggregate versions of the functions
