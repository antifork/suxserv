#
# Copyright 2002 Barnaba Marcello <vjt@azzurra.org>
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 
# 2a. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
# 2b. Redistribution in binary form requires specific prior written
#     authorization of the maintainer.
# 
# 3. All advertising materials mentioning features or use of this
#    software must display the following acknowledgement:
#    This product includes software developed by Barnaba Marcello.
# 
# 4. The names of the maintainer, developers and contributors may not be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE MAINTAINER, DEVELOPERS AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
# THE DEVELOPERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

CC=gcc
RM=/bin/rm
LDFLAGS=-L/usr/lib `pkg-config --libs glib-2.0` `pkg-config --libs gthread-2.0` -pg
#LDFLAGS=-L/usr/lib `pkg-config --libs glib-2.0` `pkg-config --libs gthread-2.0`
CFLAGS=-I../include `pkg-config --cflags glib-2.0` `pkg-config --cflags gthread-2.0` -Winline -Wall -g -pg -fprofile-arcs
#CFLAGS=-I../include `pkg-config --cflags glib-2.0` `pkg-config --cflags gthread-2.0` -Winline -Wall -fomit-frame-pointer -funroll-loops -O3

ME=sux

SOURCES = main.c parse.c dispatch.c match.c table.c s_err.c network.c log.c threads.c

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
threads.o: threads.c \
    ../include/sux.h ../include/threads.h
