#ifndef __sux_h__
#define __sux_h__

#include <glib.h>
#include "os.h"
#include "table.h"

#define NICKLEN		30
#define USERLEN		10
#define HOSTLEN 	64
#define INFOLEN		50
#define	TOPICLEN	307
#define	CHANNELLEN      32
#define KEYLEN		32

#define UMODE_o 0x0001
#define UMODE_i 0x0002
#define UMODE_h 0x0004
#define UMODE_A 0x0008
#define UMODE_x 0x0010
#define UMODE_a 0x0020
#define UMODE_z 0x0040
#define UMODE_r 0x0080
#define UMODE_R 0x0100
#define UMODE_S 0x0200

typedef struct user
{
    struct user *next;
    char nick[NICKLEN + 1];
    char username[USERLEN + 1];
    char host[HOSTLEN + 1];	
    char virthost[HOSTLEN + 1];
    char gcos[INFOLEN + 1];
    char server[NICKLEN + 1];
    time_t ts;
    short mode;
    TABLE_T *channels;
    TABLE_T *identified_chans;
    TABLE_T *identified_nicks;
    short invalid_pw_count;
    time_t invalid_pw_time;
    time_t lastmemosend;
    time_t lastnickreg;
    time_t flood_time;
    short floodlev;
    short motd, version;
} User;

#define CMODE_i 0x0001
#define CMODE_m 0x0002
#define CMODE_n 0x0004
#define CMODE_p 0x0008
#define CMODE_s 0x0010
#define CMODE_t 0x0020
#define CMODE_k 0x0040
#define CMODE_l 0x0080
#define CMODE_r 0x0100
#define CMODE_j 0x0200
#define CMODE_R 0x0400
#define CMODE_c 0x0800
#define CMODE_O 0x1000
#define CMODE_M 0x2000
typedef struct
{
   long limit;
   unsigned short mode;
   char key[KEYLEN + 1];
} Mode;

typedef struct channel
{
   struct channel *next;
   char topic[TOPICLEN + 1];
   char topic_nick[NICKLEN + 1];
   time_t topic_time;
   time_t channelts;
   Mode mode;
   TABLE_T *bans;
   TABLE_T *users;
   char chname[CHANNELLEN + 1];
} Channel;

#endif /* __sux_h__ */
