# Generated automatically from Makefile.in by configure.
CC=gcc
RM=/bin/rm
LDFLAGS=-pg
CFLAGS=-I. -Wall -g -pg -fprofile-arcs -O3
ME=sux

SOURCES = memory.c main.c dbuf.c parse.c dispatch.c match.c usertable.c s_err.c

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
	
dbuf.c:./services.h ./dbuf.h ./memory.h
dispatch.c:./services.h ./h.h ./main.h ./usertable.h ./numeric.h
main.c:./services.h ./dbuf.h ./main.h ./parse.h ./usertable.h
memory.c:./services.h ./memory.h
parse.c:./h.h ./services.h ./main.h ./parse.h ./match.h
s_err.c:./services.h ./numeric.h
usertable.c:./services.h ./memory.h ./main.h ./usertable.h ./match.h
