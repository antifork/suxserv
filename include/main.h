#ifndef __main_h__
#define __main_h__

#define BUFSIZE 512

#ifndef MAIN
# define EXTERN extern
#else
# define EXTERN
#endif

typedef struct mydata
{
    time_t boot;
    GIOChannel *handle;

    gchar host[HOSTLEN],
    	name[HOSTLEN],
	info[INFOLEN],
	pass[NICKLEN];

    User *uplink;
	
    gushort port;

    GSource *send_tag,
    	*recv_tag,
	*err_tag;

    GString *sendQ, *recvQ;

    GMutex *ctx_mutex, *time_mutex;
    GCond *ctx_cond;
    GPollFD fds[4];
    GMainContext *ctx;

    GCond *readbuf_cond;
    GMutex *readbuf_mutex;
    GMutex *writebuf_mutex;
    GMutex *me_mutex;

    time_t now;
} MyData;

EXTERN MyData me;

#define NOW			(me.now)

EXTERN gboolean net_thr_running;
EXTERN gboolean parse_thr_running;

EXTERN GSource *g_input_add(GIOChannel *, GIOCondition, GIOFunc);

#undef EXTERN
#endif
