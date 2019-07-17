Testing
=======

# Summary

This directory contains the test harnesses for both SQL and C code within this extension. To avoid interoperability issues with external support libraries such as pgTAP, this extension implements its own tests.

For the tests to complete successfully, they need a PostgreSQL server running and availble on local port 5432. Please also ensure that the user running the test has a `.pgpass` file allowing it to connect to the database cluster as the postgres. Superuser is needed as the framework creates its own scratch database `__pgp_testing__`, which it drops afterwards.

The test harness assumes that the extension has already been built and installed on the server.

# Writing Tests

SQL function test cases are stored in JSON files with the following structure:

```bash
{
    "test number":"test_case",
    ...
    ...
}
```

The test case is a valid SQL state that is expected to return TRUE if the test case passes. If a test case fails, the user will be notified which test case has failed.

New test cases should be expressed in a way that does not rely on other functions being operational (each test should be independent and assume that all other functions do not pass tests)
