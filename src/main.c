/* insert a (C) notice here */

#define MAIN
#include "sux.h"
#include "main.h"
#include "parse.h"
#include "network.h"
#include "log.h"
#include "h.h"

extern gint errno;

static void setup_allocators(void);
static void setup_fds(void);
static void setup_mutexes(void);
static void setup_signals(void);

static int start_net_thread(void);
static int start_parse_thread(void);
static int start_sig_thread(void);

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
	GThread *net_thr, *parse_thr, *sig_thr;
	GError *err = NULL;
	pid_t pid = fork();
	gint *received_signal = NULL;

	switch(pid)
	{
	    case 0:
		/* child */		
		g_thread_init(NULL);
		
		setup_allocators();
		setup_netbuf();
		setup_tables();
		setup_fds();
		setup_mutexes();
		setup_signals();
		
		log_set_irc_wrapper();
		
		net_thr = g_thread_create((GThreadFunc)start_net_thread, NULL, FALSE, &err);
		if(net_thr == NULL)
		{
		    exit(-1);
		}

		parse_thr = g_thread_create((GThreadFunc)start_parse_thread, NULL, FALSE, &err);
		if(parse_thr == NULL)
		{
		    exit(-1);
		}

		sig_thr = g_thread_create((GThreadFunc)start_sig_thread, NULL, FALSE, &err);
		if(sig_thr == NULL)
		{
		    exit(-1);
		}

		received_signal = g_async_queue_pop(me.sig_queue);
		if(*received_signal != 0)
		{
		    g_critical("Received signal %d, quitting", *received_signal);
		}

		exit(0);

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

static void setup_signals(void)
{
    sigset_t signal_set;
    
    sigfillset(&signal_set);
    pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
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

    me.sig_queue = g_async_queue_new();
}

static int start_net_thread(void)
{
    gint max_priority;
    gint timeout;
    gboolean some_ready;
    gint nfds, allocated_nfds = 4;

    g_mutex_lock(me.ctx_mutex);
    me.ctx = g_main_context_new();
    g_main_context_ref(me.ctx);
    g_mutex_unlock(me.ctx_mutex);

    g_message_syslog("Services booting, pid: %d", getpid());

    me.recv_tag = g_source_add(me.handle, G_IO_IN | G_IO_ERR | G_IO_HUP, 
	    (GIOFunc) net_receive_callback);
    me.send_tag = g_source_add(me.handle, G_IO_OUT | G_IO_ERR,
	    (GIOFunc) net_send_callback);
    me.err_tag = g_source_add(me.handle, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
	    (GIOFunc) net_err_callback);

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

	if(nfds || timeout != 0)
	{
	    if(poll((struct pollfd *)me.fds, nfds, timeout) < 0 && errno != EINTR)
	    {
		g_errno_critical("poll()");
	    }
	}

	g_main_context_check(me.ctx, max_priority, me.fds, nfds);
	g_main_context_dispatch(me.ctx);

	g_main_context_release(me.ctx);

	g_mutex_lock(me.ctx_mutex);

    } while(TRUE);

    return 0;
}

static int start_parse_thread(void)
{
    gchar **strings;
    gint i, count;

    while(TRUE)
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

    g_thread_exit(NULL);

    return 0;
}

static int start_sig_thread(void)
{
    sigset_t signal_set;
    gint *sig = g_new0(gint, 1);

    for(;;)
    {
	sigfillset(&signal_set);
	sigwait(&signal_set, sig);

	switch(*sig)
	{
	    case SIGTERM:
	    case SIGQUIT:
	    case SIGHUP:
	    case SIGINT:
		g_async_queue_push(me.sig_queue, sig);
		g_thread_exit(NULL);

		break;
	    case SIGPIPE:
	    default:
		break;
	}
    }
}
