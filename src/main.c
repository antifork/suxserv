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

#define MAIN
#include "sux.h"
#include "main.h"
#include "parse.h"
#include "network.h"
#include "log.h"
#include "h.h"
#include "threads.h"

extern gint errno;

static void setup_allocators(void);
static void setup_fds(void);
static void setup_signals(void);

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

    log_set_tty_wrapper();

    if((me.handle = connect_server(me.host, SUX_UPLINK_PORT)))
    {
	switch(0)//fork())
	{
	    case 0:
		/* child */		
		g_thread_init(NULL);
		
		setup_mutexes();
		setup_netbuf();
		setup_signals();
		setup_fds();
		setup_allocators();
		setup_tables();
		
		log_set_irc_wrapper();

		spawn_threads();
		wait_for_termination();
		clean_exit();

		return 0;

	    case -1:
		/* error */
		g_errno_critical("fork()");

		return -1;

	    default:
		/* father */
		g_message("Services are daemonizing");
		exit(EXIT_SUCCESS);

		return 0;
	}
    }

    g_critical("Cannot connect to server %s:%d", me.host, SUX_UPLINK_PORT);

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
    
    gsl_all = g_allocator_new("GSList allocator", 1024);

    g_slist_push_allocator(gsl_all);
}

static void setup_fds(void)
{
    fflush(stdout);
    fflush(stderr);

    close(0);
    close(1);
    close(2);
}
