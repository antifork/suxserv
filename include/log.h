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
void log_set_irc_wrapper_net_thr(void);
void log_set_irc_wrapper_parse_thr(void);

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
