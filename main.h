#ifndef __main_h__
#define __main_h__

#include "dbuf.h"
#define BUFSIZE 512
#define IOBUFSIZE 8192

#ifndef MAIN
# define EXTERN extern
#else
# define EXTERN
#endif

typedef struct mydata
{
    time_t boot;
    int sock;
    char host[HOSTLEN];
    char name[HOSTLEN];
    char info[INFOLEN];
    char pass[PASSLEN];
    char uplink[HOSTLEN];
    unsigned short port;
    
    char buffer[BUFSIZE+1];
    struct DBuf sendQ;
    struct DBuf recvQ;
} MyData;

EXTERN MyData me;
EXTERN void fatal(char *fmt, ...);

#undef EXTERN
#endif
