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

#define	CHFL_CHANOP     0x0001	/* Channel operator */
#define	CHFL_VOICE      0x0002	/* the power to speak */
#define	CHFL_DEOPPED 	0x0004	/* deopped by us, modes need to be bounced */
#define	CHFL_BAN	0x0008	/* ban channel flag */

#define	MODE_CHANOP	CHFL_CHANOP
#define	MODE_VOICE	CHFL_VOICE
#define	MODE_DEOPPED  	CHFL_DEOPPED
#define	MODE_PRIVATE  	0x00008
#define	MODE_SECRET   	0x00010
#define	MODE_MODERATED  0x00020
#define	MODE_TOPICLIMIT 0x00040
#define	MODE_INVITEONLY 0x00080
#define	MODE_NOPRIVMSGS 0x00100
#define	MODE_KEY	0x00200
#define	MODE_BAN	0x00400
#define	MODE_LIMIT	0x00800
#define MODE_REGISTERED	0x01000
#define MODE_REGONLY	0x02000
#define MODE_NOCOLOR	0x04000
#define MODE_OPERONLY   0x08000
#define MODE_MODREG     0x10000
#define MODE_LISTED	0x20000
#define MODE_NONICKCHG	0x40000

typedef struct
{
   gulong limit;
   gushort mode;
   gchar key[KEYLEN + 1];
} Mode;

typedef struct chanmember
{
    User *u;
    int flags;
} ChanMember;

typedef struct channel
{
   gchar chname[CHANNELLEN + 1];
   gchar topic[TOPICLEN + 1];
   gchar topic_nick[NICKLEN + 1];
   time_t topic_time;
   time_t ts;
   Mode mode;
   GSList *bans;
   GSList *members;
} Channel;

typedef struct slink
{
    union
    {
	User *u;
	Channel *c;
	gchar *cp;
    } value;
    gint flags;
} SLink;

#define SUX_MODULE	"Sux Core Services"
#define SUX_RELEASE	"0.02"
#define SUX_VERSION	SUX_MODULE " " SUX_RELEASE

#define SUX_SERV_NAME	"services.azzurra.org"
#define SUX_PASS	"codio"

#define SUX_UPLINK	"twisted.vejnet.org"

#endif /* __sux_h__ */
