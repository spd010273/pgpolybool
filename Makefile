EXTENSION       = pgpolybool
EXTVERSION      = $(shell grep default_version $(EXTENSION).control | \
                  sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/" )
PG_CONFIG      ?= pg_config
PGLIBDIR        = $(shell $(PG_CONFIG) --libdir)
PGINCLUDEDIR    = $(shell $(PG_CONFIG) --includedir-server )
CC              = gcc
LIBS            = -lm
PG_CFLAGS       = -I$(PGINCLUDEDIR) -Isrc/lib/
MODULE_big      = pgpolybool
SRCS            = $(wildcard src/lib/*.c) $(wildcard src/*.c)
OBJS            = $(SRCS:.c=.o)
PGXS            = $(shell $(PG_CONFIG) --pgxs)
EXTRA_CLEAN     = src/*.o src/*.so *.so *.o sql/$(EXTENSION)--$(EXTVERSION).sql

all: sql/$(EXTENSION)--$(EXTVERSION).sql

sql/$(EXTENSION)--$(EXTVERSION).sql: $(sort $(wildcard sql/functions/*.sql)) $(sort $(wildcard sql/casts/*.sql)) $(sort $(wildcard sql/permissions/*.sql))
	cat $^ > $@

# Add -DDEBUG to enable DEBUG output of log level DEBUG
PG_CPPFLAGS     = -DDEBUG -g $(PG_CFLAGS)
DATA            = $(wildcard sql/updates/*--*.sql) sql/$(EXTENSION)--$(EXTVERSION).sql

include $(PGXS)
