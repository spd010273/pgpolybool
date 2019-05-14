## __override_path_center

### Prototype:

VOID __override_path_center()

### Description:

This function hooks the path_center stub located at src/backend/utils/adt/geo_ops.c with the patt to point cast function in pgpolybool.

This hook involves inserting an x86-64 long jump at the address of path_center. This jump is performed with the following:

```asm
PUSH <address low>
MOV [ESP + 4] <address high>
RET
```

The address of path_center is located using the dynamic linkers symbol table.

The functionality of this function is further documented in the source at `src/lib/hook.c`

### Error Conditions:

* Emits an error on failure to open PostgreSQLs symbol table using the Real Time Dynamic Linker.
* Emits an error on failure to locate the address of path_center.
* Emits an error on failure to set page permissions to allow writing.
* Emits an error on failure to reset page permissions after the hook has been installed.
