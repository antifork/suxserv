CC=gcc
RM=/bin/rm
LDFLAGS=-L/usr/lib `pkg-config --libs glib-2.0` `pkg-config --libs gthread-2.0` -pg
CFLAGS=-I../include `pkg-config --cflags glib-2.0` `pkg-config --cflags gthread-2.0` -Wall -g -pg -fprofile-arcs
ME=sux

SOURCES = main.c parse.c dispatch.c match.c table.c s_err.c network.c log.c

OBJECTS = $(SOURCES:.c=.o)

all: $(ME)

main: main.c
	$(CC) -o main $<

build: all

clean:
#remove objs
	$(RM) -f $(OBJECTS) 

#remove cores and stale sources
	$(RM) -f *~ $(ME).core core $(ME) parse.c

#remove gperf crap
	$(RM) -f *.da a.out gmon.out

parse.c: parse.gperf
	gperf -tGDW cmds -N hash_get_cmd -K cmd parse.gperf > parse.c

.c.o:
	$(CC) $(CFLAGS) $(INCLUDEDIR) -c $<

sux:    $(OBJECTS)
	$(CC) ${LDFLAGS} -o $(ME) $(OBJECTS)
	ln -sf $(ME) a.out
	
dispatch.o: dispatch.c \
    ../include/sux.h ../include/h.h ../include/main.h \
    ../include/table.h ../include/numeric.h
main.o: main.c \
    ../include/sux.h ../include/main.h ../include/parse.h \
    ../include/table.h ../include/log.h
memory.o: memory.c \
    ../include/sux.h ../include/table.h
parse.o: parse.c \
    ../include/h.h ../include/sux.h ../include/main.h \
    ../include/parse.h ../include/match.h
s_err.o: s_err.c \
    ../include/sux.h ../include/numeric.h
table.o: table.c \
    ../include/sux.h ../include/table.h ../include/main.h \
    ../include/match.h
network.o: network.c \
    ../include/sux.h ../include/log.h
log.o: log.c \
    ../include/sux.h ../include/network.h ../include/main.h \
    ../include/log.h
