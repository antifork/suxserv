/* insert a (C) notice here */

#define MAIN
#include "sux.h"
#include "main.h"
#include "parse.h"
#include "network.h"
#include "log.h"

extern gint errno;

static void signal_handler(gint);

gint main(gint argc, gchar **argv)
{
    GMainLoop *main_loop;

    strcpy(me.name, "services.azzurra.org");
    strcpy(me.info, "SuxServices 0.001");
    strcpy(me.pass, "codio");
    strcpy(me.host, "homes.vejnet.org");

    me.port = htons(6667);

    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);

    log_set_tty_wrapper();

    if((me.handle = connect_server(me.host, me.port)))
    {
	pid_t pid = 0; //fork();

	switch(pid)
	{
	    case 0:
		/* child */

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

		send_out("PASS %s :TS", me.pass);
		send_out("CAPAB NOQUIT SSJOIN UNCONNECT NICKIP TSMODE");
		send_out("SVINFO 5 3 0 :%lu", time(NULL));
		send_out("SERVER %s 1 :%s", me.name, me.info);
	
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
		g_message("services are daemonizing [pid %d]", pid);
		exit(EXIT_SUCCESS);

		return 0;
	}
    }

    g_critical("cannot connect to server %s:%d", me.host, me.port);

    return -1;
}

static void signal_handler(gint sig)
{
    g_critical_syslog("received signal %d, quitting", sig);
}
