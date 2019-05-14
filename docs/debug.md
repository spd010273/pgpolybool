# Debugging tips and tricks:

Make sure to build postgresql from source with the following modifications to `src/include/pg_config_manual.h`:

```bash
#define USE_VALGRIND
#define RANDOMIZE_ALLOCATED_MEMORY
```

PostgreSQL can be compiled from source with:

```bash
yum install gcc make readline-devel zlib-devel libtermcap-devel python-devel \
            tcl-devel openssl-devel krb5-devel e2fsprogs-devel gettext \
            libxml2-devel pam-devel uuid-devel openldap-devel perl-devel \
            valgrind
useradd postgres
mkdir -p /usr/local/pgsql
mkdir -p /var/lib/pgsql/<version>/data
chown postgres.postgres /usr/local/pgsql
chown -R postgres.postgres /var/lib/pgsql
cd ~
git clone https://github.com/postgres/postgres.git
cd ~/postgres
git checkout origin/<release>
./configure
make clean && make
make install
/usr/lib/pgsql/bin/pg_ctl initdb -D /var/lib/pgsql/<version>/data
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

is properly symlinked to `/usr/local/pgsql/bin/pg_config` and that a make install of the extension places the shared library in `/usr/local/pgsql/lib/`
