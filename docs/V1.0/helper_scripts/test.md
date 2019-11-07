## test/run_tests.pl

### Description:

This is the test harness for the pgpolybool extension. This script runs the tests stored in test/sql/. Upon completion, the script will emit what tests have failed.
There is also a test harness for internal C functions (currently just the RB BST). These are not included in run_tests.pl, but can be compiled and ran separately.
For more information about writing tests, see test/README_test.md
