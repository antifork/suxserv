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
    gchar nick[NICKLEN + 1];
    gchar username[USERLEN + 1];
    gchar host[HOSTLEN + 1];	
    gchar virthost[HOSTLEN + 1];
    gchar gcos[INFOLEN + 1];
    gchar server[NICKLEN + 1];
    time_t ts;
    gshort mode;
    GSList *channels;
    gshort invalid_pw_count;
    time_t invalid_pw_time;
    time_t lastmemosend;
    time_t lastnickreg;
    time_t flood_time;
    gshort floodlev;
    gshort motd, version;
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
   gulong limit;
   gushort mode;
   gchar key[KEYLEN + 1];
} Mode;

typedef struct channel
{
   gchar topic[TOPICLEN + 1];
   gchar topic_nick[NICKLEN + 1];
   time_t topic_time;
   time_t channelts;
   Mode mode;
   GSList *bans;
   GList *users;
   gchar chname[CHANNELLEN + 1];
} Channel;

#endif /* __sux_h__ */
