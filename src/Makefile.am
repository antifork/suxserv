bin_PROGRAMS = sux

#sux_LDADD = \
#    @GLIB_LIBS@

CLEANFILES = parse.c

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
	-I$(top_srcdir)/include \
	$(GLIB_CFLAGS) \
	-DSYSCONFDIR=\""$(sysconfdir)"\"

sux_SOURCES = \
    conf.c \
    dispatch.c \
    log.c \
    main.c \
    match.c \
    network.c \
    s_err.c \
    table.c \
    threads.c \
    parse.c

parse.c: parse.gperf
	gperf -tGDW cmds -N hash_get_cmd -K cmd parse.gperf > parse.c

