/* log wrappers */

#include "sux.h"
#include "network.h"
#include "main.h"
#include "log.h"

void __sux_irc_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message, GMainLoop *main_loop)
{
    gchar *err_type = NULL;
    gint save_errno = errno;
    
    if(log_level & G_LOG_LEVEL_ERROR)
	err_type = "Error";
    else if(log_level & G_LOG_LEVEL_CRITICAL)
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

    if(log_level & G_LOG_LEVEL_SYSLOG)
    {
	if(log_level & G_LOG_LEVEL_ERRNO)
	{
	    syslog(LOG_NOTICE, "%s: %s: %s", log_domain, message, strerror(save_errno));
	}
	else
	{
	    syslog(LOG_NOTICE, "%s: %s", log_domain, message);
	}
	
    }
    else
    {
	if(log_level & G_LOG_LEVEL_ERRNO)
	{
	    send_out(":%s GLOBOPS :[%s] %s: %s (%s)",
		    me.name, log_domain, err_type, message, strerror(save_errno));
	}
	else
	{
	    send_out(":%s GLOBOPS :[%s] %s: %s",
		    me.name, log_domain, err_type, message);
	}
    }

    if(log_level & G_LOG_LEVEL_ERROR)
    {
	g_main_loop_quit(main_loop);
	abort();
    }

    if(log_level & G_LOG_LEVEL_CRITICAL)
    {
	g_main_loop_quit(main_loop);
	exit(EXIT_FAILURE);
    }
}

void __sux_tty_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message)
{
    gchar *err_type = NULL;
    gint save_errno = errno;
    
    if(log_level & G_LOG_LEVEL_ERROR)
	err_type = "Error";
    else if(log_level & G_LOG_LEVEL_CRITICAL)
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

    if(log_level & G_LOG_LEVEL_ERRNO)
    {
	fprintf(stderr, "[%s] %s: %s (%s)\n",
		log_domain, err_type, message, strerror(save_errno));
    }
    else
    {
	fprintf(stderr, "[%s] %s: %s\n",
		log_domain, err_type, message);
    }

    if(log_level & G_LOG_LEVEL_ERROR)
    {
	fprintf(stderr, "aborting ...\n");
	abort();
    }

    if(log_level & G_LOG_LEVEL_CRITICAL)
    {
	fprintf(stderr, "exiting ...\n");
	exit(EXIT_FAILURE);
    }
}
