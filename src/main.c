/* insert a (C) notice here */

#define MAIN
#include "sux.h"
#include "main.h"
#include "parse.h"
#include "usertable.h"
#include "network.h"

extern gint errno;

static void exit_func(gint);
static void stderr_fatal(char *, ...);
static void syslog_fatal(char *, ...);
static void stderr_warn(char *, ...);
static void syslog_warn(char *, ...);
static void syslog_init(void);

gint main(gint argc, gchar **argv)
{
    GMainLoop *main_loop;

    strcpy(me.name, "services.azzurra.org");
    strcpy(me.info, "SuxServices 0.001");
    strcpy(me.pass, "codio");
    strcpy(me.host, "homes.vejnet.org");

    me.port = htons(6667);

    signal(SIGINT, exit_func);
    signal(SIGHUP, exit_func);
    signal(SIGQUIT, exit_func);
    signal(SIGTERM, exit_func);

    fatal = stderr_fatal;
    warn = stderr_warn;
    
    if((me.handle = connect_server(me.host, me.port)))
    {
	pid_t pid = fork();

	switch(pid)
	{
	    case 0:
		/* child */
		syslog_init();
		tables_init();
		
		fatal = syslog_fatal;
		warn = syslog_warn;
		
		main_loop = g_main_loop_new(NULL, TRUE);

		g_io_add_watch(me.handle,
			G_IO_IN | G_IO_ERR | G_IO_HUP, (GIOFunc) net_receive_callback, NULL);

		g_io_add_watch(me.handle,
			G_IO_ERR | G_IO_HUP | G_IO_NVAL, (GIOFunc) net_err_callback, NULL);

		me.send_tag = g_io_add_watch(me.handle,
			G_IO_OUT | G_IO_ERR, (GIOFunc) net_send_callback, NULL);

		send_out("PASS %s :TS", me.pass);
		send_out("CAPAB NOQUIT SSJOIN UNCONNECT NICKIP TSMODE");
		send_out("SVINFO 5 3 0 :%lu", time(NULL));
		send_out("SERVER %s 1 :%s", me.name, me.info);
	
		g_main_loop_run(main_loop);
	
		return 0;

	    case -1:
		/* error */
		fatal("fork()");

		return -1;

	    default:
		/* father */
		warn("services are daemonizing [pid \1%d\1]\n", pid);
		
		exit(EXIT_SUCCESS);

		return 0;
	}
    }

    fatal("cannot connect to server %s:%d", me.host, me.port);

    return 0;
}

static void stderr_fatal(gchar *fmt, ...)
{
    va_list ap;
    gint save_errno = errno;

    va_start (ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, ": %s\n", save_errno ? strerror(save_errno) : "Terminated.");
    exit(EXIT_FAILURE);
}

static void syslog_fatal(gchar *fmt, ...)
{
    va_list ap;
    gint save_errno = errno;
    gchar msgbuf[BUFSIZ];

    va_start(ap, fmt);
    vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
    va_end(ap);
    
    syslog(LOG_ERR, "%s: %s", msgbuf, save_errno ? strerror(save_errno) : "Terminated.");

    syslog(LOG_NOTICE, "syslog session closed");
    closelog();

    exit(EXIT_FAILURE);
}

static void stderr_warn(gchar *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void syslog_warn(gchar *fmt, ...)
{
    va_list ap;
    gchar msgbuf[BUFSIZ];

    va_start(ap, fmt);
    vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
    va_end(ap);

    syslog(LOG_NOTICE, "%s", msgbuf);
}

#ifdef G_CAN_INLINE
G_INLINE_FUNC
#endif
static void syslog_init(void)
{
    openlog("sux", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_NOTICE, "syslog session opened");
}

static void exit_func(gint sig)
{
    errno = 0;
    fatal("received signal %d, quitting", sig);
}
