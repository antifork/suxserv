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
static void setup_fds(void);
static void setup_mutexes(void);

static int start_net_thread(void);
static int start_parse_thread(void);

gint main(gint argc, gchar **argv)
{
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
	GThread *net_thr, *parse_thr;
	GError *err = NULL;
	pid_t pid = 0 ;//fork();

	switch(pid)
	{
	    case 0:
		/* child */		
		g_thread_init(NULL);
		
		setup_signals();
		setup_allocators();
		setup_netbuf();
		setup_tables();
		setup_fds();
		setup_mutexes();
		
		net_thr = g_thread_create((GThreadFunc)start_net_thread, NULL, TRUE, &err);
		if(net_thr == NULL)
		{
		    g_critical(err->message);
		    return 0;
		}

		parse_thr = g_thread_create((GThreadFunc)start_parse_thread, NULL, TRUE, &err);
		if(parse_thr == NULL)
		{
		    g_critical(err->message);
		    return 0;
		}

		g_thread_join(net_thr);
		g_thread_join(parse_thr);
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

GSource *g_input_add(GIOChannel *handle, GIOCondition cond, GIOFunc callback)
{
    GSource *gs;

    gs = g_io_create_watch(handle, cond);
    g_source_set_priority(gs, G_PRIORITY_DEFAULT);
    g_source_set_callback(gs, (GSourceFunc) callback, NULL, NULL);

    g_mutex_lock(me.ctx_mutex);
    g_source_attach(gs, me.ctx);
    g_mutex_unlock(me.ctx_mutex);

    g_source_unref(gs);

    return gs;
}
    
static void signal_handler(gint sig)
{
    g_critical("Received signal %d (%s), quitting", sig, g_strsignal(sig));
    g_critical_syslog("Received signal %d (%s), quitting", sig, g_strsignal(sig));
}

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

static void setup_fds(void)
{
    fflush(stdout);
    fflush(stderr);

    close(0);
    /*close(1);*/
    /*close(2);*/
}

static void setup_mutexes(void)
{
    me.ctx_mutex = g_mutex_new();
    me.ctx_cond = g_cond_new();
    
    me.readbuf_mutex = g_mutex_new();
    me.readbuf_cond = g_cond_new();
    me.writebuf_mutex = g_mutex_new();

    me.time_mutex = g_mutex_new();
}

static void g_main_context_poll(GMainContext *ctx, gint timeout,
	gint priority, GPollFD *fds, gint n_fds)
{
    GPollFunc poll_func;
    
    g_mutex_lock(me.ctx_mutex);
    poll_func = g_main_context_get_poll_func(ctx);
    g_mutex_unlock(me.ctx_mutex);

    if(n_fds || timeout != 0)
    {
	if((*poll_func)(fds, n_fds, timeout) < 0 && errno != EINTR)
	{
	    g_errno_critical("poll()");
	}
    }
}

static int start_net_thread(void)
{
    gint max_priority;
    gint timeout;
    gboolean some_ready;
    gint nfds, allocated_nfds = 4;

    log_set_irc_wrapper_net_thr();

    g_mutex_lock(me.ctx_mutex);
    me.ctx = g_main_context_new();
    g_main_context_ref(me.ctx);
    g_mutex_unlock(me.ctx_mutex);

    g_message_syslog("Services booting, pid: %d", getpid());

    me.recv_tag = g_input_add(me.handle, G_IO_IN | G_IO_ERR | G_IO_HUP, 
	    (GIOFunc) net_receive_callback);
    me.send_tag = g_input_add(me.handle, G_IO_OUT | G_IO_ERR,
	    (GIOFunc) net_send_callback);
    me.err_tag = g_input_add(me.handle, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
	    (GIOFunc) net_err_callback);

    net_thr_running = TRUE;
    
    nego_start();
    
    do
    {
	g_mutex_unlock(me.ctx_mutex);

	if(!g_main_context_acquire(me.ctx))
	{
	    gboolean got_own;

	    g_mutex_lock(me.ctx_mutex);
	    got_own = g_main_context_wait(me.ctx, me.ctx_cond, me.ctx_mutex);

	    if(!got_own)
	    {
		g_mutex_unlock(me.ctx_mutex);
		continue;
	    }
	}

	some_ready = g_main_context_prepare(me.ctx, &max_priority);
	nfds = g_main_context_query(me.ctx, max_priority, &timeout, me.fds, allocated_nfds);

	g_main_context_poll(me.ctx, timeout, max_priority, me.fds, nfds);
	g_main_context_check(me.ctx, max_priority, me.fds, nfds);
	g_main_context_dispatch(me.ctx);

	g_main_context_release(me.ctx);

	g_mutex_lock(me.ctx_mutex);

    } while(net_thr_running);

    net_shutdown(me.handle);

    g_source_destroy(me.recv_tag);
    g_source_destroy(me.err_tag);
    if(me.send_tag != NULL)
    {
        g_source_destroy(me.send_tag);
    }

    g_io_channel_unref(me.handle);

    g_main_context_unref(me.ctx);

    return 0;
}

static int start_parse_thread(void)
{
    gchar **strings;
    gint i, count;
	
    log_set_irc_wrapper_parse_thr();

    parse_thr_running = TRUE;
    
    while(parse_thr_running)
    {
	g_mutex_lock(me.readbuf_mutex);
	if(!me.recvQ->len)
	{
	    g_cond_wait(me.readbuf_cond, me.readbuf_mutex);
	}

	if(me.recvQ->str[me.recvQ->len - 1] != '\n')
	{
	    /* incomplete string */
	    strings = my_g_strsplit(me.recvQ->str, '\n', &count);
	    me.recvQ = g_string_assign(me.recvQ, strings[--count]);
	}
	else
	{
	    strings = my_g_strsplit(me.recvQ->str, '\n', &count);
	    me.recvQ = g_string_erase(me.recvQ, 0, -1);
	}
	g_mutex_unlock(me.readbuf_mutex);

	g_mutex_lock(me.time_mutex);
	NOW = time(NULL);
	g_mutex_unlock(me.time_mutex);

	for(i = 0; i < count; i++)
	{
	    parse(strings[i]);
	}

	g_strfreev(strings);
    }

    return 0;
}
