PG_CONFIG   	?= pg_config
PGLIBDIR 		= $(shell $(PG_CONFIG) --libdir)
PGINCLUDEDIR	= $(shell $(PG_CONFIG) --includedir-server )
CC 				= gcc
LIBS			= -lm
PG_CFLAGS	    = -I$(PGINCLUDEDIR) -Isrc/lib/
MODULE_big 	    = src/pgpolybool
PGXS			= $(shell $(PG_CONFIG) --pgxs)
EXTRA_CLEAN		= src/*.o src/*.so *.so *.o
PG_CPPFLAGS		= -DDEBUG -g $(PG_CFLAGS)
OBJS            = src/lib/connector.o src/lib/polygon.o src/lib/segment.o src/lib/contour.o src/lib/util.o src/lib/pqueue.o src/lib/martinez.o src/pgpolybool.o
#SRC				= src/lib/connector.c src/lib/polygon.c src/lib/segment.c src/lib/contour.c src/lib/util.c src/lib/pqueue.c src/lib/martinez.c src/pgpolybool.c
#HDR				= src/lib/connector.h src/lib/polygon.h src/lib/segment.h src/lib/contour.h src/lib/util.h src/lib/pqueue.h src/lib/martinez.h
DATA			= sql/pgpolybool.sql

include $(PGXS)

all: src/pgpolybool.so
src/pgpolybool.so: src/lib/pqueue.o src/lib/util.o src/lib/connector.o src/lib/segment.o src/lib/contour.o src/lib/polygon.o src/lib/martinez.o src/pgpolybool.o
src/pgpolybool.o: src/pgpolybool.c
src/lib/pqueue.o: src/lib/pqueue.c
src/lib/util.o: src/lib/util.c
src/lib/connector.o: src/lib/connector.c
src/lib/segment.o: src/lib/segment.c
src/lib/contour.o: src/lib/contour.c
src/lib/polygon.o: src/lib/polygon.c
src/lib/martinez.o: src/lib/martinez.c


#src/lib/pqueue.o:
#	$(CC) -o src/lib/pqueue.o src/lib/pqueue.c $(LIBS) $(PG_CPPFLAGS) -I$(PGINCLUDEDIR)
#
#src/lib/segment.o:
#	$(CC) -o src/lib/segment.o src/lib/segment.c $(LIBS) $(PG_CPPFLAGS) -I$(PGINCLUDEDIR)
#
#src/lib/util.o:
#	$(CC) -o src/lib/util.o src/lib/util.c $(LIBS) $(PG_CPPFLAGS) -I$(PGINCLUDEDIR)
#
#src/lib/contour.o:
#	$(CC) -o src/lib/contour.o src/lib/contour.c $(LIBS) $(PG_CPPFLAGS) -I$(PGINCLUDEDIR)
#
#src/lib/polygon.o:
#	$(CC) -o src/lib/polygon.o src/lib/polygon.c $(LIBS) $(PG_CPPFLAGS) -I$(PGINCLUDEDIR)
#
#src/lib/martinez.o:
#	$(CC) -o src/lib/martinez.o src/lib/martinez.c $(LIBS) $(PG_CPPFLAGS) -I$(PGINCLUDEDIR)
#
#src/pgpolybool.o: $(SRC)
#	$(CC) -o src/pgpolybool.o $(HDR) $(SRC) $(CPPFLAGS) -I$(PGINCLUDEDIR)
#src/pgpolybool.so: $(OBJS)
#	$(CC) -shared -fPIC -o src/pgpolybool.so $(OBJS) $(CPPFLAGS) -I$(PGINCLUDEDIR)
