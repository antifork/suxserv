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

/* log wrappers */

#include "sux.h"
#include "network.h"
#include "main.h"
#include "log.h"
#include "threads.h"

void __sux_gen_log_handler_error(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    syslog(LOG_NOTICE, "%s: Fatal: %s", log_domain, message);

    abort();
}

void __sux_gen_log_handler_message_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    syslog(LOG_NOTICE, "%s: Message: %s", log_domain, message);
}


void __sux_irc_log_handler_critical_errno(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    G_CONST_RETURN gchar *errstr;
    gint *zero = g_new0(gint, 1);
    
    errstr = g_strerror(errno);
    send_out(":%s GLOBOPS :[%s] Critical: %s (%s)",
	    me.name, log_domain, message, errstr);
    send_out("SQUIT %s :%s (%s)", me.name, message, errstr);

    push_signal(zero);
}

void __sux_irc_log_handler_critical_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    syslog(LOG_NOTICE, "%s: Critical: %s", log_domain, message);
    gint *zero = g_new0(gint, 1);

    push_signal(zero);
}

void __sux_irc_log_handler_generic(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    gchar *err_type;

    if(log_level & G_LOG_LEVEL_CRITICAL)
	err_type = "Critical";
    else if(log_level & G_LOG_LEVEL_WARNING)
	err_type = "Warning";
    else if(log_level & G_LOG_LEVEL_MESSAGE)
	err_type = "Message";
    else if(log_level & G_LOG_LEVEL_INFO)
	err_type = "Info";
    else if(log_level & G_LOG_LEVEL_DEBUG)
	err_type = "Debug";
    else
	err_type = "Unknown";

    send_out(":%s GLOBOPS :[%s] %s: %s", me.name, log_domain, err_type, message);

    if(log_level & G_LOG_LEVEL_CRITICAL)
    {
	gint *zero = g_new0(gint, 1);
	send_out("SQUIT %s :%s", me.name, message);
	push_signal(zero);
    }
}

void __sux_tty_log_handler_critical_errno(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    G_CONST_RETURN gchar *errstr = g_strerror(errno);

    g_fprintf(stderr, "(process %d) [%s] Critical **: %s (%s)\n", getpid(), log_domain, message, errstr);
    exit(-1);
}

void __sux_tty_log_handler_critical_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    syslog(LOG_NOTICE, "%s: Critical: %s", log_domain, message);
    exit(-1);
}

void __sux_tty_log_handler_generic(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    gchar *err_type;

    if(log_level & G_LOG_LEVEL_CRITICAL)
	err_type = "Critical";
    else if(log_level & G_LOG_LEVEL_WARNING)
	err_type = "Warning";
    else if(log_level & G_LOG_LEVEL_MESSAGE)
	err_type = "Message";
    else if(log_level & G_LOG_LEVEL_INFO)
	err_type = "Info";
    else if(log_level & G_LOG_LEVEL_DEBUG)
	err_type = "Debug";
    else
	err_type = "Unknown";

    g_fprintf(stderr, "(process %d) [%s] %s **: %s\n", getpid(), log_domain, err_type, message);

    if(log_level & G_LOG_LEVEL_CRITICAL)
    {
	exit(-1);
    }
}

void log_set_irc_wrapper(void)
{
    openlog("sux", LOG_NDELAY | LOG_NOWAIT | LOG_PID, LOG_DAEMON);
    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_gen_log_handler_error, NULL);

    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_MESSAGE | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_gen_log_handler_message_syslog, NULL);

    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_ERRNO_CRITICAL | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_irc_log_handler_critical_errno, NULL);
    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_CRITICAL | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_irc_log_handler_critical_syslog, NULL);
    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL |
	    G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_FLAG_RECURSION, 
	    (GLogFunc) __sux_irc_log_handler_generic, NULL);
}

void log_set_tty_wrapper(void)
{
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_gen_log_handler_error, NULL);

    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_MESSAGE | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_gen_log_handler_message_syslog, NULL);

    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_ERRNO_CRITICAL | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_tty_log_handler_critical_errno, NULL);
    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_CRITICAL | G_LOG_FLAG_RECURSION,
	    (GLogFunc) __sux_tty_log_handler_critical_syslog, NULL);
    
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | 
	    G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_FLAG_RECURSION, 
	    (GLogFunc) __sux_tty_log_handler_generic, NULL);
}
