/* log wrappers */

#include "sux.h"
#include "network.h"
#include "main.h"
#include "log.h"

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
    G_CONST_RETURN gchar *errstr = g_strerror(errno);

    send_out(":%s GLOBOPS :[%s] Critical: %s (%s)",
	    me.name, log_domain, message, errstr);
    
    send_out("SQUIT %s :%s (%s)", me.name, message, errstr);
    STOP_RUNNING();
}

void __sux_irc_log_handler_critical_syslog(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    syslog(LOG_NOTICE, "%s: Critical: %s", log_domain, message);
    STOP_RUNNING();
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
	send_out("SQUIT %s :%s", me.name, message);
	STOP_RUNNING();
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
