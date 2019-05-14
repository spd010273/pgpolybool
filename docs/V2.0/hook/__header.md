# Hooking Functions

These functions hook PostgreSQL builtins to override behavior. These functions must be called to hook the function on a running server.

This functionality can be made permenant by adding pgpolybool.so to `shared_preload_libraries` in postgresql.conf
