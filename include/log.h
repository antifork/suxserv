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

#ifndef __log_h__
#define __log_h__

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif

typedef enum
{
    G_LOG_LEVEL_ERRNO_CRITICAL		=	(1 << (G_LOG_LEVEL_USER_SHIFT)),
    G_LOG_LEVEL_SYSLOG_CRITICAL		=	(1 << (G_LOG_LEVEL_USER_SHIFT + 1)),
    G_LOG_LEVEL_SYSLOG_MESSAGE		=	(1 << (G_LOG_LEVEL_USER_SHIFT + 2))
} MyGlogLevelFlags;

#define G_LOG_DOMAIN		"Core services"
#ifdef G_HAVE_ISO_VARARGS
#define g_errno_critical(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERRNO_CRITICAL, __VA_ARGS__)
#define g_critical_syslog(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_CRITICAL, __VA_ARGS__)
#define g_message_syslog(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_MESSAGE, __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_errno_critical(format...)	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERRNO_CRITICAL, format)
#define g_critical_syslog(format...)	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_CRITICAL, format)
#define g_message_syslog(format...)	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_MESSAGE, format)
#else
static void g_errno_critical(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_ERRNO_CRITICAL, format, args);
    va_end(args);
}
static void g_critical_syslog(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_CRITICAL, format, args);
    va_end(args);
}
static void g_message_syslog(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_SYSLOG_MESSAGE, format, args);
    va_end(args);
}
#endif

void log_set_tty_wrapper(void);
void log_set_irc_wrapper(void);

void __sux_gen_log_handler_error(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);
void __sux_gen_log_handler_message_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);


void __sux_tty_log_handler_errno_critical(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);
void __sux_tty_log_handler_critical_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);
void __sux_tty_log_handler_generic(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);

void __sux_irc_log_handler_errno_critical(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);
void __sux_irc_log_handler_critical_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);
void __sux_irc_log_handler_generic(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);

#endif
