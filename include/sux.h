/*
 * Copyright 2002 Barnaba Marcello <vjt@azzurra.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2a. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 2b. Redistribution in binary form requires specific prior written
 *     authorization of the maintainer.
 * 
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *    This product includes software developed by Barnaba Marcello.
 * 
 * 4. The names of the maintainer, developers and contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE MAINTAINER, DEVELOPERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE DEVELOPERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __sux_h__
#define __sux_h__

#define G_THREADS_ENABLED
#include <glib.h>
#include <glib/gprintf.h>
#include "os.h"
#include "table.h"

#define NICKLEN		30
#define USERLEN		10
#define HOSTLEN 	64
#define INFOLEN		50
#define	TOPICLEN	307
#define	CHANNELLEN      32
#define KEYLEN		32
#define	PASSWDLEN 	63

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

typedef struct user User;
struct user
{
    gchar name[NICKLEN + 1];
    gchar username[USERLEN + 1];
    gchar host[HOSTLEN + 1];	
    gchar info[INFOLEN + 1];
    User *server;
    time_t ts;
    gshort mode;
    GSList *channels;
    gshort flags;
};

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

typedef struct mode Mode;
struct mode
{
   gulong limit;
   gushort mode;
   gchar key[KEYLEN + 1];
};

typedef struct chanmember ChanMember;
struct chanmember
{
    User *u;
    int flags;
};

typedef struct channel Channel;
struct channel
{
   gchar chname[CHANNELLEN + 1];
   gchar topic[TOPICLEN + 1];
   gchar topic_nick[NICKLEN + 1];
   time_t topic_time;
   time_t ts;
   Mode mode;
   GSList *bans;
   GSList *members;
};

typedef struct slink SLink;
struct slink
{
    union
    {
	User *u;
	Channel *c;
	gchar *cp;
    } value;
    gint flags;
};

#define CAPAB_NOQUIT  0x0002 /* Supports NOQUIT */
#define CAPAB_BURST   0x0008 /* server supports BURST command */
#define CAPAB_UNCONN  0x0010 /* server supports UNCONNECT */
#define CAPAB_DKEY    0x0020 /* server supports dh-key exchange using "DKEY" */
#define CAPAB_ZIP     0x0040 /* server supports gz'd links */
#define CAPAB_DOZIP   0x0080 /* output to this link shall be gzipped */
#define CAPAB_DODKEY  0x0100 /* do I do dkey with this link? */
#define CAPAB_NICKIP  0x0200 /* IP in the NICK line? */
#define CAPAB_TSMODE  0x0400 /* MODE's parv[2] is chptr->channelts for channel mode */

#define NEEDED_CAPABS	(CAPAB_NOQUIT | CAPAB_BURST | CAPAB_UNCONN | CAPAB_NICKIP)

#define seconds * 1000
#define minutes * 60 seconds
#define second seconds
#define minute minutes

#define PING_FREQUENCY 10 minutes
#define PING_TIMEOUT 30 seconds

#define FLAGS_USERBURST	   0x01   /* server in nick/channel netburst */
#define FLAGS_TOPICBURST   0x02   /* server in topic netburst */
#define FLAGS_SOBSENT      0x04   /* we've sent an SOB, just have to 
			           * send an EOB */
#define FLAGS_EOBRECV      0x08   /* we're waiting on an EOB */
#define FLAGS_BURST	   (FLAGS_USERBURST | FLAGS_TOPICBURST)

#define FLAGS_AWAY         0x10

#define SUX_MODULE	"Sux Core Services"
#define SUX_RELEASE	"0.2"
#define SUX_VERSION	SUX_MODULE " " SUX_RELEASE

#define SUX_SERV_NAME	"services.azzurra.org"
#define SUX_PASS	"codio"

#define SUX_UPLINK_HOST	"twisted.vejnet.org"
#define SUX_UPLINK_PORT	6667
#define SUX_UPLINK_NAME "server.dal.net"

#define SUX_CUR_TS	5
#define SUX_MIN_TS	3

#ifdef G_INLINE_FUNC
#undef G_INLINE_FUNC
#endif

#define G_INLINE_FUNC __inline__


#endif /* __sux_h__ */
