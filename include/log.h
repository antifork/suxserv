#ifndef __log_h__
#define __log_h__

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif

#define G_LOG_LEVEL_ERRNO	1 << (G_LOG_LEVEL_USER_SHIFT + 1)
#define G_LOG_LEVEL_SYSLOG	1 << (G_LOG_LEVEL_USER_SHIFT + 2)

#define G_LOG_DOMAIN		"Core services"
#ifdef G_HAVE_ISO_VARARGS
#define g_errno_critical(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERRNO, __VA_ARGS__)
#define g_critical_syslog(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_SYSLOG, __VA_ARGS__)
#define g_error_syslog(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_SYSLOG, __VA_ARGS__)
#define g_message_syslog(...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_SYSLOG, __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_errno_critical(format...)	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERRNO, format)
#define g_critical_syslog(format...)	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_SYSLOG, format)
#define g_error_syslog(format...)	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_SYSLOG, format)
#define g_message_syslog(format...)		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_SYSLOG, format)
#else
static void g_errno_critical(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERRNO, format, args);
    va_end(args);
}
static void g_critical_syslog(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_SYSLOG, format, args);
    va_end(args);
}
static void g_error_syslog(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_SYSLOG, format, args);
    va_end(args);
}
static void g_message_syslog(const gchar *format, ...)
{
    va_list args;
    va_start (args, format);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_SYSLOG, format, args);
    va_end(args);
}
#endif

#define log_set_tty_wrapper()	g_log_set_handler(G_LOG_DOMAIN,		\
					G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,	\
					(GLogFunc) __sux_tty_log_handler, NULL);

#define	log_init_syslog()	openlog("sux", LOG_NDELAY | LOG_NOWAIT | LOG_PID, LOG_DAEMON);

#define log_set_irc_wrapper(lp)	g_log_set_handler(G_LOG_DOMAIN,		\
					G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,	\
					(GLogFunc) __sux_irc_log_handler, lp);

void __sux_irc_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message, GMainLoop *main_loop);

void __sux_tty_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message);

#endif
