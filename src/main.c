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

gint main(gint argc, gchar **argv)
{
    if(!GLIB_CHECK_VERSION(2, 2, 0))
    {
	g_critical("GLib version 2.2.0 or above is required");
	exit(EXIT_FAILURE);
    }

    if(argc > 1)
    {
	log_set_irc_wrapper();

	strcpy(me.name, SUX_SERV_NAME);
	strcpy(me.pass, SUX_PASS);
	strcpy(me.info, SUX_VERSION);

	if((me.handle = connect_server(SUX_UPLINK_HOST, SUX_UPLINK_PORT)))
	{
	    start_master_thread();
	}
	else
	{
	    g_critical_syslog("Cannot connect to server %s:%d", SUX_UPLINK_HOST, SUX_UPLINK_PORT);
	    exit(EXIT_FAILURE);
	}
    }
    else
    {
	gint child_pid;
	GError *err;
	
	log_set_tty_wrapper();

	gchar *child_argv[3];

	child_argv[0] = argv[0];
	child_argv[1] = "-d";
	child_argv[2] = NULL;
	
	if(g_spawn_async_with_pipes(NULL, child_argv, NULL,  G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
		    G_SPAWN_STDOUT_TO_DEV_NULL| G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_DO_NOT_REAP_CHILD,
		    NULL, NULL, &child_pid, NULL, NULL, NULL, &err))
	{
	    g_message("Services started (master thread id: %d)", child_pid);

	    exit(EXIT_SUCCESS);
	}
	else
	{
	    g_critical("Failed to start master thread: %s", err->message);
	    g_error_free(err);

	    exit(EXIT_FAILURE);
	}
    }

    return EXIT_FAILURE;
}
