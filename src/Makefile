CC=gcc
RM=/bin/rm
LDFLAGS=-pg -L/usr/lib -lglib-2.0
CFLAGS=-I../include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -Wall -g -pg -fprofile-arcs -DDEBUG
ME=sux

SOURCES = main.c parse.c dispatch.c match.c usertable.c s_err.c network.c log.c

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
	gperf -tDW cmds -N hash_get_cmd -K cmd parse.gperf > parse.c

.c.o:
	$(CC) $(CFLAGS) $(INCLUDEDIR) -c $<

sux:    $(OBJECTS)
	$(CC) ${LDFLAGS} -o $(ME) $(OBJECTS)
	ln -sf $(ME) a.out
	
dispatch.o: dispatch.c \
    ../include/sux.h ../include/h.h ../include/main.h \
    ../include/usertable.h ../include/numeric.h
main.o: main.c \
    ../include/sux.h ../include/main.h ../include/parse.h \
    ../include/usertable.h ../include/log.h
memory.o: memory.c \
    ../include/sux.h ../include/table.h
parse.o: parse.c \
    ../include/h.h ../include/sux.h ../include/main.h \
    ../include/parse.h ../include/match.h
s_err.o: s_err.c \
    ../include/sux.h ../include/numeric.h
usertable.o: usertable.c \
    ../include/sux.h ../include/table.h ../include/main.h \
    ../include/usertable.h ../include/match.h
network.o: network.c \
    ../include/sux.h ../include/log.h
log.o: log.c \
    ../include/sux.h ../include/network.h ../include/main.h \
    ../include/log.h
