CC=gcc
RM=/bin/rm
LDFLAGS=-pg -L/usr/lib -lglib-2.0
CFLAGS=-I. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -Wall -g -pg -fprofile-arcs -DDEBUG
ME=sux


SOURCES = memory.c main.c parse.c dispatch.c match.c usertable.c s_err.c network.c

OBJECTS = $(SOURCES:.c=.o)

all: $(ME)

main: main.c
	$(CC) -o main $<

build: all

clean:
	$(RM) -f $(OBJECTS) *~ $(ME).core core $(ME) parse.c *.da a.out gmon.out

parse.c: parse.gperf
	gperf -tDW cmds -N hash_get_cmd -K cmd parse.gperf > parse.c

.c.o:
	$(CC) $(CFLAGS) $(INCLUDEDIR) -c $<

sux:    $(OBJECTS)
	$(CC) ${LDFLAGS} -o $(ME) $(OBJECTS)
	ln -sf $(ME) a.out
	
dispatch.o: dispatch.c ./sux.h ./h.h ./main.h ./usertable.h ./numeric.h
main.o: main.c ./sux.h ./main.h ./parse.h ./usertable.h
memory.o: memory.c ./sux.h ./memory.h
parse.o: parse.c ./h.h ./sux.h ./main.h ./parse.h ./match.h
s_err.o: s_err.c ./sux.h ./numeric.h
usertable.o: usertable.c ./sux.h ./memory.h ./main.h ./usertable.h ./match.h
network.o: network.c ./sux.h
