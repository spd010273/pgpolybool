pgpolybool
==========

# Summary

pgpolybool is a geometric library supporting various operations in R2 (2D) Euclidean space using the native PostgreSQL geometric types. This project seeks to round out the geometric feature set in the PostgreSQL codebase
as well as provide helper functions for common spatial / geometric tasks such as:

* Boolean logic for Polygon types
* Centroid-based rotation / translation
* Convex and concave hull algorithms for point fields

# Getting Started

## Prerequisites:

This extension requires the following:

* PostgreSQL 9.4 or greater
* gcc, make, and PostgreSQL development libraries

## Installing:

Prior to starting, checkout this repository, perform a build with

```bash
make
```

Verify that pg_config is available in the environment's PATH with:

```bash
which pg_config
```

This should be present in /usr/pgsql-<version>/bin/pg_config for RHEL/CentOS and /usr/bin/pg_config for Debian/Ubuntu. Once this executable is properly linked, run:

```bash
su - postgres -c make install
```

This will install pgpolybool to PostgreSQL's extensions and lib folders. Installation can be completed by logging into the database and running:

```SQL
CREATE EXTENSION pgpolybool;
```

# Versions

See CHANGELOG.md

# License

pgpolybool is released under the PostgreSQL license. For more details about this licence, please see LICENSE

# Acknowledgments

* Francisco Martinez, Antonio Reuda, and Francisco Feito for developing this algorithm, it is extremely versatile and fast. It excelled in cases where both the Vatti algorithm or Greiner-Hormann failed or could not be easily modified to handle collinearity edge cases. The authors also released their original work to the public domain.
* Sean Connelly for his work at (http://sean.cm/a/polygon-clipping-pt2). This post contains a very well illustrated and articulated walkthrough of the algorithm's operation. There is also a interactive tool for playing around with polygon inputs to the algorithm.
* PostgreSQL Global Development Group, for their finely maintained project, exceptional documentation, and enthusiastic user base.
* Richard Davies for encouragements, jokes, insights and tricks in C/C++.
* My employer, Nead Werx, Inc., for granting me the time to bring this implementation to PostgreSQL.
