PG_CONFIG   	?= pg_config
PGLIBDIR 		= $(shell $(PG_CONFIG) --libdir)
PGINCLUDEDIR	= $(shell $(PG_CONFIG) --includedir-server )
CC 				= gcc
LIBS			= -lm
PG_CFLAGS	    = -I$(PGINCLUDEDIR) -Isrc/lib/
#MODULES			= src/pgpolybool
MODULE_big 	    = pgpolybool
SRCS			= src/lib/poly_funcs.c src/lib/box_funcs.c src/lib/lseg_funcs.c src/lib/connector.c src/lib/polygon.c src/lib/segment.c src/lib/contour.c src/lib/util.c src/lib/dlpq.c src/lib/martinez.c src/lib/polyprocessing.c src/pgpolybool.c
OBJS            = $(SRCS:.c=.o)
PGXS			= $(shell $(PG_CONFIG) --pgxs)
EXTRA_CLEAN		= src/*.o src/*.so *.so *.o

# Add -DDEBUG to enable DEBUG output of log level DEBUG
PG_CPPFLAGS		= -DDEBUG -g $(PG_CFLAGS)
DATA			= sql/pgpolybool--1.0.sql
EXTENSION       = pgpolybool
include $(PGXS)
