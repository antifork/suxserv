/* insert a (C) notice here */

#define MAIN
#include "sux.h"
#include "main.h"
#include "parse.h"
#include "usertable.h"
#include "network.h"

extern gint errno;

static void exit_func(gint);

gint main(gint argc, gchar **argv)
{
    GMainLoop *main_loop;

    strcpy(me.name, "services.azzurra.org");
    strcpy(me.info, "SuxServices 0.001");
    strcpy(me.pass, "codio");
    strcpy(me.host, "homes.vejnet.org");
    me.port = htons(6667);
    signal(SIGINT, exit_func);
    
    if((me.handle = connect_server(me.host, me.port)))
    {
	tables_init();
	main_loop = g_main_loop_new(NULL, TRUE);

	g_io_add_watch(me.handle,
		G_IO_IN | G_IO_ERR | G_IO_HUP, (GIOFunc) net_receive_callback, NULL);
	g_io_add_watch(me.handle,
		G_IO_ERR | G_IO_HUP | G_IO_NVAL, (GIOFunc) net_err_callback, NULL);
	me.send_tag = g_io_add_watch(me.handle,
		    G_IO_OUT | G_IO_ERR, (GIOFunc) net_send_callback, NULL);

	send_out("PASS %s :TS", me.pass);
	send_out("CAPAB NOQUIT SSJOIN UNCONNECT NICKIP TSMODE");
	send_out("SVINFO 5 3 0 :%lu", time(NULL));
	send_out("SERVER %s 1 :%s", me.name, me.info);
	
	g_main_loop_run(main_loop);
	
	return 0;
    }

    fatal("cannot connect to server %s:%d", me.host, me.port);

    return 0;
}

void fatal(gchar *fmt, ...)
{
    va_list ap;
    gint save_errno = errno;

    va_start (ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, ": %s\n", save_errno ? strerror(save_errno) : "Terminated.");
    exit(EXIT_FAILURE);
}


static void exit_func(gint sig)
{
    fprintf(stderr, "received signal %d, quitting ..\n",
	    sig);
    exit(0);
}
