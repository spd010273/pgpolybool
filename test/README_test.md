Testing
=======

This directory contains the test harnesses for both SQL and C code within this extension. To avoid interoperability issues with external support libraries such as pgTAP, this extension implements its own tests.

For the tests to complete successfully, they need a PostgreSQL server running and availble on local port 5432. Please also ensure that the user running the test has a `.pgpass` file allowing it to connect to the database cluster as postgresql. Superuser is needed as the framework creates its own scratch database `__pgp_testing__`, which it drops afterwards.
