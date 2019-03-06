Debugging tips and tricks:

Make sure to build postgresql from source with the following modifications to src/include/pg_config_manual.h:

```bash
#define USE_VALGRIND
#define RANDOMIZE_ALLOCATED_MEMORY
```

Once complete, start postgresql with the following:

```bash
sudo su - postgres
valgrind --suppressions=/tmp/valgrind.supp \
         -v \
         --trace-children=yes \
         --read-var-info=yes \
         --leak-check=full \
         --show-leak-kinds=all \
         /usr/local/pgsql/bin/postmaster -D /var/lib/pgsql/<version>/data \
         &> /tmp/valgrind.out
```

Ensure that 

```bash
which pg_config
```

is properly symlinked to /usr/local/pgsql/bin/pg_config and that a make install of the extension places the shared library in /usr/local/pgsql/lib/
