/* insert a (C) notice here */

#define MAIN
#include "sux.h"
#include "main.h"
#include "parse.h"
#include "network.h"
#include "log.h"
#include "h.h"

extern gint errno;

static void signal_handler(gint);
static void setup_signals(void);
static void setup_allocators(void);

gint main(gint argc, gchar **argv)
{
    GMainLoop *main_loop;

    if(!GLIB_CHECK_VERSION(2, 0, 0))
    {
	g_critical("GLib version 2.0.0 or above is required");
	exit(EXIT_FAILURE);
    }

    strcpy(me.name, SUX_SERV_NAME);
    strcpy(me.pass, SUX_PASS);
    strcpy(me.info, SUX_VERSION);
    strcpy(me.host, SUX_UPLINK_HOST);

    me.port = htons(SUX_UPLINK_PORT);

    log_set_tty_wrapper();

    if((me.handle = connect_server(me.host, me.port)))
    {
	pid_t pid = 0; //fork();

	switch(pid)
	{
	    case 0:
		/* child */
		
		setup_signals();
		setup_allocators();
		setup_netbuf();

		fflush(stdout);
		fflush(stderr);

		close(0);
		close(1);
		/*close(2);*/
	
		main_loop = g_main_loop_new(NULL, TRUE);

		tables_init();
		
		log_set_irc_wrapper();

		g_message_syslog("Services booting, pid: %d", getpid());

		me.recv_tag = g_io_add_watch(me.handle,
			G_IO_IN | G_IO_ERR | G_IO_HUP, (GIOFunc) net_receive_callback, NULL);

		me.send_tag = g_io_add_watch(me.handle,
			G_IO_OUT | G_IO_ERR, (GIOFunc) net_send_callback, NULL);

		me.err_tag = g_io_add_watch(me.handle,
			G_IO_ERR | G_IO_HUP | G_IO_NVAL, (GIOFunc) net_err_callback, NULL);

		nego_start();
	
		START_RUNNING();

		do
		{
		    g_main_context_iteration(NULL, TRUE);
		} while(IS_RUNNING());

		net_shutdown(me.handle);

		g_source_remove(me.recv_tag);
		g_source_remove(me.err_tag);
		if(me.send_tag != -1)
		    g_source_remove(me.send_tag);

		g_io_channel_unref(me.handle);

		g_main_loop_unref(main_loop);
	
		return 0;

	    case -1:
		/* error */
		g_errno_critical("fork()");

		return -1;

	    default:
		/* father */
		g_message("Services are daemonizing [pid %d]", pid);
		exit(EXIT_SUCCESS);

		return 0;
	}
    }

    g_critical("Cannot connect to server %s:%d", me.host, me.port);

    return -1;
}

static void signal_handler(gint sig)
{
    g_critical("Received signal %d (%s), quitting", sig, g_strsignal(sig));
    g_critical_syslog("Received signal %d (%s), quitting", sig, g_strsignal(sig));
}

/*
static void dummy_signal(gint sig)
{
    signal(sig, dummy_signal);
}
*/

static void setup_signals(void)
{
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);
}

static void setup_allocators(void)
{
    static GAllocator *gsl_all;
    
    gsl_all = g_allocator_new("GSList allocator", 4096);

    g_slist_push_allocator(gsl_all);
}
