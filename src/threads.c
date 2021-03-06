/*
 * Copyright 2002 Barnaba Marcello <vjt@azzurra.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2a. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 2b. Redistribution in binary form requires specific prior written
 *     authorization of the maintainer.
 * 
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *    This product includes software developed by Barnaba Marcello.
 * 
 * 4. The names of the maintainer, developers and contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE MAINTAINER, DEVELOPERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE DEVELOPERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sux.h"
#include "main.h"
#include "network.h"
#include "log.h"
#include "threads.h"
#include "parse.h"

#define ALLOCATED_NFDS	4

void nego_start(void);
static void network_thread(void);
static void parser_thread(void);
static void signal_thread(void);
static void master_thread(void);

static void join_threads(void);
static G_INLINE_FUNC void my_g_main_context_iteration(void);

static GThread *network_thread_ptr, *parser_thread_ptr, *signal_thread_ptr;
static GMainContext *ctx;
static GAsyncQueue *sig_queue, *thread_queue;

GLOBAL_RUN_DECLARE();

void start_master_thread(void)
{
    g_thread_init(NULL);

    setup_mutexes();
    setup_netbuf();
    setup_tables();
		
    log_set_irc_wrapper();

    master_thread();
}

void join_threads(void)
{
    STOP_RUNNING();

    g_cond_signal(me.readbuf_cond);
    
    g_thread_join(network_thread_ptr);
    g_thread_join(parser_thread_ptr);

    /*
     * here we have only this thread running
     */

    while(me.send_tag && me.recv_tag)
    {
	my_g_main_context_iteration();
    }

    net_shutdown(me.handle);
}

void setup_mutexes(void)
{
    me.tag_mutex = g_mutex_new();
    
    me.readbuf_mutex = g_mutex_new();
    me.readbuf_cond = g_cond_new();
    me.writebuf_mutex = g_mutex_new();

    sig_queue = g_async_queue_new();

    GLOBAL_RUN_INIT();
}

static void setup_signals(void)
{
    sigset_t signal_set;
    
    sigfillset(&signal_set);
    pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
}

void wait_for_termination(void)
{
    gint *received_signal;
    
    received_signal = g_async_queue_pop(sig_queue);
    if(*received_signal != 0)
    {
	g_critical("Received signal %d (%s), quitting",
		*received_signal, g_strsignal(*received_signal));
    }
}

G_INLINE_FUNC void my_g_main_context_iteration(void)
{
    gint max_priority;
    gint timeout;
    gint nfds;
    static GPollFD fds[ALLOCATED_NFDS];

    g_main_context_prepare(ctx, &max_priority);
    nfds = g_main_context_query(ctx, max_priority, &timeout, fds, ALLOCATED_NFDS);

    if(nfds || timeout != 0)
    {
	if(poll((struct pollfd *)fds, nfds, timeout) < 0 && errno != EINTR)
	{
	    g_errno_critical("poll()");
	}
    }

    g_main_context_check(ctx, max_priority, fds, nfds);
    g_main_context_dispatch(ctx);

    return;
}

void master_thread(void)
{
    GError *err = NULL;
    gpointer thr_id;

    setup_signals();
    thread_queue = g_async_queue_new();

    signal_thread_ptr = g_thread_create_full((GThreadFunc)signal_thread,
	    NULL, 0, FALSE, TRUE, G_THREAD_PRIORITY_LOW, &err);
    if(signal_thread_ptr == NULL)
    {
	exit(EXIT_FAILURE);
    }
    thr_id = g_async_queue_pop(thread_queue);
    g_message_syslog("Initialized signal thread, pid: %d", GPOINTER_TO_INT(thr_id));

    network_thread_ptr = g_thread_create_full((GThreadFunc)network_thread,
	    NULL, 0, TRUE, TRUE, G_THREAD_PRIORITY_NORMAL, &err);
    if(network_thread_ptr == NULL)
    {
	exit(EXIT_FAILURE);
    }
    thr_id = g_async_queue_pop(thread_queue);
    g_message_syslog("Initialized network thread, pid: %d", GPOINTER_TO_INT(thr_id));

    parser_thread_ptr = g_thread_create_full((GThreadFunc)parser_thread,
	    NULL, 0, TRUE, TRUE, G_THREAD_PRIORITY_NORMAL, &err);
    if(parser_thread_ptr == NULL)
    {
	exit(EXIT_FAILURE);
    }
    thr_id = g_async_queue_pop(thread_queue);
    g_message_syslog("Initialized parser/dispatcher thread, pid: %d", GPOINTER_TO_INT(thr_id));

    g_async_queue_unref(thread_queue);

    wait_for_termination();
    join_threads();
}

static void network_thread(void)
{
    THREAD_RUN_DECLARE();

    ctx = g_main_context_new();
    g_main_context_ref(ctx);

    time(&me.boot);

    me.recv_tag = g_source_add(me.handle, G_IO_IN | G_IO_ERR | G_IO_HUP, 
	    (GIOFunc) net_receive_callback);
    me.send_tag = g_source_add(me.handle, G_IO_OUT | G_IO_ERR,
	    (GIOFunc) net_send_callback);
    me.err_tag = g_source_add(me.handle, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
	    (GIOFunc) net_err_callback);

    nego_start();

    g_async_queue_push(thread_queue, GINT_TO_POINTER(getpid()));
 
    THREAD_RUN_CHECK();

    while(THREAD_IS_RUNNING())
    {
	my_g_main_context_iteration();

	THREAD_RUN_CHECK();
    }

    g_thread_exit(NULL);
}

static void parser_thread(void)
{
    const gint STRINGS_PER_CYCLE = 2048;
    gchar **strings = g_new0(gchar *, STRINGS_PER_CYCLE);
    GString *read_data = g_string_sized_new(READBUFSZ);
    gint i, count;
    THREAD_RUN_DECLARE();

    g_async_queue_push(thread_queue, GINT_TO_POINTER(getpid()));

    THREAD_RUN_CHECK();

    while(THREAD_IS_RUNNING())
    {
	g_mutex_lock(me.readbuf_mutex);
	if(!me.recvQ->len)
	{
	    /*g_thread_yield();*/
	    g_cond_wait(me.readbuf_cond, me.readbuf_mutex);
	}

	if(!me.recvQ->len)
	{
	    /*
	     * possibly are we terminated ?
	     */
	    g_mutex_unlock(me.readbuf_mutex);
	    
	    THREAD_RUN_CHECK();
	    continue;
	}

	read_data = g_string_assign(read_data, me.recvQ->str);
	if(my_g_strsplit(read_data->str, '\n', STRINGS_PER_CYCLE, &count, strings) == FALSE)
	{
	    /* there is something to stick back into the recvQ */
	    me.recvQ = g_string_assign(me.recvQ, strings[--count]);
	}
	else
	{
	    me.recvQ = g_string_erase(me.recvQ, 0, -1);
	}
	g_mutex_unlock(me.readbuf_mutex);

	NOW = time(NULL);

	for(i = 0; i < count; i++)
	{
	    parse(strings[i]);
	}

	THREAD_RUN_CHECK();
    }

    g_thread_exit(NULL);

}

static void signal_thread(void)
{
    sigset_t signal_set;
    gint *sig = g_new0(gint, 1);
    
    sigfillset(&signal_set);

    g_async_queue_push(thread_queue, GINT_TO_POINTER(getpid()));

    for(;;)
    {
	sigwait(&signal_set, sig);

	switch(*sig)
	{
	    case SIGSEGV:
		g_message("Aieeeee!!! Ship sinks !!! Women and childrens first !!!");
		g_mutex_lock(me.readbuf_mutex);
		if(me.recvQ->len)
		    g_message("Input buffer: '%s'", me.recvQ->str);
		g_mutex_unlock(me.readbuf_mutex);

	    case SIGTERM:
	    case SIGQUIT:
	    case SIGHUP:
	    case SIGINT:
		g_async_queue_push(sig_queue, sig);
		g_thread_exit(NULL);

		break;

	    case SIGABRT:
		abort();

		break;

	    case SIGPIPE:
	    default:
		break;
	}

	sigaddset(&signal_set, *sig);
    }

    g_thread_exit(NULL);
}

G_INLINE_FUNC GSource *g_source_add(GIOChannel *handle, GIOCondition cond, GIOFunc callback)
{
    GSource *gs;

    gs = g_io_create_watch(handle, cond);
    g_source_set_priority(gs, G_PRIORITY_DEFAULT);
    g_source_set_callback(gs, (GSourceFunc) callback, NULL, NULL);

    g_source_attach(gs, ctx);

    return gs;
}

G_INLINE_FUNC GSource *g_timeout_source_add(guint interval, GSourceFunc callback, gpointer user_data)
{
    GSource *gs;

    gs = g_timeout_source_new(interval);
    g_source_set_priority(gs, G_PRIORITY_DEFAULT);
    g_source_set_callback(gs, (GSourceFunc) callback, user_data, NULL);

    g_source_attach(gs, ctx);

    return gs;
}

G_INLINE_FUNC void g_source_del(GSource *gs)
{
    g_return_if_fail(gs != NULL);
    
    g_source_destroy(gs);
    g_source_unref(gs);
}

void push_signal(gint *signum)
{
    g_async_queue_push(sig_queue, signum);
}
