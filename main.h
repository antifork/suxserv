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

    char host[HOSTLEN];
    char name[HOSTLEN];
    char info[INFOLEN];
    char pass[NICKLEN];
    char uplink[HOSTLEN];
    unsigned short port;

    int send_tag;

} MyData;

EXTERN MyData me;
EXTERN void fatal(char *fmt, ...);

#undef EXTERN
#endif
