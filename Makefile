# Generated automatically from Makefile.in by configure.
CC=gcc
RM=/bin/rm
LDFLAGS=-pg
CFLAGS=-I. -Wall -g -pg -fprofile-arcs -O3
ME=sux

SOURCES = memory.c main.c dbuf.c parse.c dispatch.c match.c table.c

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
