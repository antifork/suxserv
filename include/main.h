#ifndef __main_h__
#define __main_h__

#define BUFSIZE 512
#define IOBUFSIZE 32768

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

    gint send_tag,
    	recv_tag,
	err_tag;

} MyData;

EXTERN MyData me;

#define RUNNING_DECLARE()	EXTERN	gboolean __is_running
#define START_RUNNING()		__is_running = TRUE
#define STOP_RUNNING()		__is_running = FALSE
#define IS_RUNNING()		(__is_running == TRUE)

RUNNING_DECLARE();

#undef EXTERN
#endif
