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

#include "sux.h"
#include "h.h"
#include "main.h"
#include "table.h"
#include "numeric.h"
#include "log.h"
#include "match.h"
#include "network.h"
#include "parse.h"
#include "threads.h"

#define DUMMY return 0;

static void add_user_to_channel(User *, Channel *, guint);
static gboolean remove_user_from_channel(Channel *, User *);
static gboolean remove_channel_from_user_chanlist(Channel *, User *);

REMOTE_TABLE_INSTANCE(user);
REMOTE_TABLE_INSTANCE(channel);

REMOTE_MEMPOOL_INSTANCE(cmembers);
REMOTE_MEMPOOL_INSTANCE(links);

static struct
{
    gshort capabs;
    gchar passwd[PASSWDLEN + 1];
    gchar name[PASSWDLEN + 1];

    GSource *ping_tag;
    time_t firsttime;
    gshort flags;
    
} uplink;

static gboolean ping_timeout(void)
{
    /* ping timeout */
    g_critical("Ping Timeout");
    
    return FALSE;
}

static gboolean send_ping(User *u)
{
    gchar ping_buf[64];
    
    if(uplink.ping_tag)
    {
	return ping_timeout();
    }

    g_sprintf(ping_buf, "PING :%s\n", me.name);

    /*
     * this saves a g_mutex_lock() and g_mutex_unlock() on every send_out()
     */
    g_mutex_lock(me.writebuf_mutex);
    me.sendQ = g_string_append(me.sendQ, ping_buf);
    g_mutex_unlock(me.writebuf_mutex);

    g_mutex_lock(me.tag_mutex);
    if(me.send_tag == NULL)
    {
	me.send_tag = g_source_add(me.handle, G_IO_OUT | G_IO_ERR, (GIOFunc)net_send_callback);
    }
    g_mutex_unlock(me.tag_mutex);
    /*
     * end of cut-&-paste from send_out()
     */
    
    uplink.ping_tag = g_timeout_source_add(PING_TIMEOUT, (GSourceFunc) ping_timeout, NULL);

    return TRUE;
}
    
void nego_start(void)
{
    send_out("PASS %s :TS", me.pass);
    send_out("CAPAB BURST NOQUIT SSJOIN UNCONNECT NICKIP TSMODE");
    send_out("SVINFO 5 3 0 :%lu", NOW);
    send_out("SERVER %s 1 :%s", me.name, me.info);

    uplink.ping_tag = g_timeout_source_add(PING_TIMEOUT, (GSourceFunc) ping_timeout, NULL);
    
    me.uplink = _TBL(user).alloc(SUX_UPLINK_NAME);
    me.table_ptr = _TBL(user).alloc(SUX_SERV_NAME);

    strcpy(me.table_ptr->info, me.info);
    me.table_ptr->ts = NOW;
    me.table_ptr->mode = 0;
}

G_INLINE_FUNC void m_message(User *u, gint parc, gchar **parv, gchar *type)
{
    gchar *srv_name;

    srv_name = strchr(parv[1], '@');
    if(srv_name)
    {
	*srv_name++ = '\0';
	if(mycmp(me.name, srv_name))
	{
	    g_message("Fake direction, message for %s@%s arrived to me (%s)",
		    parv[1], srv_name, me.name);
	    return;
	}
    }

    return;
}

gint m_notice(User *u, gint parc, gchar **parv)
{
    m_message(u, parc, parv, "NOTICE");

    return 0;
}

gint m_private(User *u, gint parc, gchar **parv)
{
    m_message(u, parc, parv, "PRIVMSG");

    return 0;
}

/*
 * m_topic
 * parv[0]: sender pfx
 * parv[1]: channel
 * parv[2]: topic setter
 * parv[3]: ts
 * parv[4]: topic text
 */
gint m_topic(User *u, gint parc, gchar **parv)
{
    Channel *c;
    gchar *chname;
    gchar *tnick;
    gint ts;
    gchar *topic;
    
    if(parc < 5)
    {
	return 0;
    }
    
    chname = parv[1];
    tnick = parv[2];
    ts = atoi(parv[3]);
    topic = parv[4];

    if(!(c = _TBL(channel).get(chname)))
    {
	g_warning("Unknown channel received on TOPIC !!");
	return 0;
    }

    strcpy(c->topic, topic);
    strcpy(c->topic_nick, tnick);
    c->ts = ts;

    return 1;
}

gint m_mode(User *u, gint parc, gchar **parv)
{
    DUMMY
}

gint m_ping(User *u, gint parc, gchar **parv)
{
    if(u == me.uplink &&
	    uplink.flags & (FLAGS_SOBSENT|~FLAGS_BURST))
    {
	gint sendqlen;
	uplink.flags &= ~FLAGS_SOBSENT;

	g_mutex_lock(me.writebuf_mutex);
	sendqlen = me.sendQ->len;
	g_mutex_unlock(me.writebuf_mutex);

	send_out("BURST %d", sendqlen);
    }

    send_out(":%s PONG :%s", me.name, parv[1]);
    return 1;
}

/*
 * PASS pwd :TS
 * CAPAB x y z
 * SERVER sname hops info
 * SVINFO ts ts_min 0 :time
 * :sname GNOTICE :Link established
 * BURST
 * NICK ..
 * NICK ..
 * PING
 * TOPIC
 * AWAY
 * PING
 * BURST sendq
 * PING
 */
gint m_pong(User *u, gint parc, gchar **parv)
{
    g_return_val_if_fail(u != NULL, -1);

    if(uplink.ping_tag)
    {
	g_source_del(uplink.ping_tag);
	uplink.ping_tag = NULL;
	return 0;
    }

    if(u != me.uplink)
    {
	return 0;
    }

    if(uplink.flags & FLAGS_USERBURST)
    {
	gchar timebuf[128];

	uplink.flags &= ~FLAGS_USERBURST;
	send_out(":%s GNOTICE :%s has processed user/channel burst, "
		"sending topic burst.", me.name, u->name);

	uplink.flags |= FLAGS_SOBSENT;
	
	/* XXX: send shun/topic burst */
	send_out(":ChanServ TOPIC #sux ChanServ %ld :last services restart: %s !", NOW, ctime_r(&NOW, timebuf));
	
	send_out("PING :%s", me.name);
    }
    else if(uplink.flags & FLAGS_TOPICBURST)
    {
	uplink.flags &= ~FLAGS_TOPICBURST;
	send_out(":%s GNOTICE :%s has processed topic burst (synched "
		"to network data).", me.name, u->name);
	send_out("PING :%s", me.name);
    }

    return 0;
}

static void send_nick_burst(void)
{
    struct my_user
    {
	gchar *name;
	gchar *umode;
	gchar *ircname;
    } my_users[] = {
	{"ChanServ", "+ixz", "Channel Sux Services"},
	{"NickServ", "+ixz", "Nickname Sux Services"},
	{"MemoServ", "+ixz", "Memo Sux Services"},
	{"OperServ", "+oixz", "Operator Sux Services"},
	{NULL, NULL, NULL}
    }, *johnny;
    
    User *u;
    Channel *c = _TBL(channel).get("#sux");

    time_t now = NOW;

    if(c == NULL)
    {
	c = _TBL(channel).alloc("#sux");
	c->ts = now;
    }

    for(johnny = my_users; johnny->name; johnny++)
    {
	u = _TBL(user).alloc(johnny->name);
	u->ts = now;
	strcpy(u->username, "service");
	strcpy(u->host, "sux.vejnet.org");
	strcpy(u->info, johnny->ircname);
	u->server = me.table_ptr;
	
	send_out("NICK %s 0 %ld %s service sux.vejnet.org %s 0 0 :%s",
		johnny->name, now, johnny->umode, me.name, johnny->ircname);
	send_out(":%s SJOIN %ld #sux +nrt  :%s", me.name, now, johnny->name);

	add_user_to_channel(u, c, 0);
    }
}

/*
 * m_nick
 * parv[0] = sender prefix
 * parv[1] = nickname
 * parv[2] = hopcount when new user; TS when nick change
 * parv[3] = TS
 * ---- new user only below ----
 * parv[4] = umode
 * parv[5] = username
 * parv[6] = hostname
 * parv[7] = server
 * parv[8] = serviceid
 * -- If NICKIP
 * parv[9] = IP
 * parv[10] = ircname
 * -- else
 * parv[9] = ircname
 * -- endif
 */
gint m_nick(User *u, gint parc, gchar **parv)
{
    if(parc > 3)
    {
	/* new user. */
	if((u = _TBL(user).get(parv[1])))
	{
	    /* uh ? we already have this ? */
	    g_critical("New user %s already exists in hash table ..",
		    parv[1]);
	    return 0;
	}

	u = _TBL(user).alloc(parv[1]);

	u->ts = strtoul(parv[3], NULL, 10);
	/* XXX: umode handling */
	strcpy(u->username, parv[5]);
	strcpy(u->host, parv[6]);
	
	u->server = _TBL(user).get(parv[7]);
	g_return_val_if_fail(u->server != NULL, -1);
	
	/* XXX: nickip handling */
	strcpy(u->info, parv[10]);
    }
    else
    {
	/* nick change */
	g_return_val_if_fail(u != NULL, -1);

	if(!_TBL(user).del(u))
	    g_critical("Cannot delete user %s", u->name);

	strcpy(u->name, parv[1]);
	u->ts = strtoul(parv[2], NULL, 10);
	_TBL(user).put(u);
    }
    
    return 1;
}

/*
 * m_error
 * parv[0] = sender prefix
 * parv[1] = message
 */
gint m_error(User *u, gint parc, gchar **parv)
{
    g_critical_syslog("%s", parv[1]);

    return 0;
}

static void list_free_atoms(gpointer *data, GMemChunk *chunk)
{
    g_return_if_fail(data != NULL);
    g_return_if_fail(chunk != NULL);

    g_mem_chunk_free(chunk, data);
}

static gboolean remove_user_from_channel(Channel *c, User *u)
{
    register GSList *johnny = c->members; /* walker */

    while(johnny)
    {
	if(((ChanMember*)johnny->data)->u == u)
	{
	    c->members = g_slist_remove_link(c->members, johnny);
	    g_mem_chunk_free(_MPL(cmembers), johnny->data);
	    g_slist_free_1(johnny);

	    g_return_val_if_fail(u->channels != NULL, -1);

	    if(c->members == NULL)
	    {
		_TBL(channel).del(c);
		_TBL(channel).destroy(c);
	    }

	    return TRUE;
	}

	johnny = g_slist_next(johnny);
    }

    return FALSE;
}

static gboolean remove_channel_from_user_chanlist(Channel *c, User *u)
{
    register GSList *johnny; /* walker */

    /* remove this channel from user chanlist */
    for(johnny = u->channels; johnny; johnny = g_slist_next(johnny))
    {
	SLink *lp = (SLink *)johnny->data;
	
	if(lp->value.c == c)
	{
	    u->channels = g_slist_remove_link(u->channels, johnny);
	    g_mem_chunk_free(_MPL(links), johnny->data);
	    g_slist_free_1(johnny);

	    return TRUE;
	}
    }

    return FALSE;
}

/*
 * m_quit
 * parv[0] = sender prefix
 * parv[1] = comment
 */
gint m_quit(User *u, gint parc, gchar **parv)
{
    register GSList *johnny; /* walker */
    register Channel *c;

    g_return_val_if_fail(u != NULL, 0);

    if(u->channels)
    {
	for(johnny = u->channels; johnny; johnny = g_slist_next(johnny))
	{
	    c = ((SLink *)johnny->data)->value.c;
	    g_return_val_if_fail(c != NULL, 0);

	    remove_user_from_channel(c, u);
	}

	g_slist_foreach(u->channels, (GFunc) list_free_atoms, _MPL(links));
	g_slist_free(u->channels);
    }
    
    _TBL(user).del(u);
    _TBL(user).destroy(u);
    return 1;
}

/*
 * m_kill 
 * parv[0] = sender prefix 
 * parv[1] = kill victim 
 * parv[2] = kill path
 */
gint m_kill(User *u, gint parc, gchar **parv)
{
    DUMMY
}

/*
 * m_part 
 * parv[0] = sender prefix 
 * parv[1] = channel
 * parv[2] = Optional part reason
 */
gint m_part(User *u, gint parc, gchar **parv)
{
    gchar *pfx = parv[0];
    gchar *chname = parv[1];

    Channel *c;
    
    g_return_val_if_fail(u != NULL, 0);
    g_return_val_if_fail(pfx != NULL, 0);
    g_return_val_if_fail(chname != NULL, 0);

    c = _TBL(channel).get(chname);
    g_return_val_if_fail(c != NULL, 0);

    /* remove this user from the channel memberlist */
    remove_user_from_channel(c, u);
    /* remove this channel from the user chanlist */
    remove_channel_from_user_chanlist(c, u);

    return 1;
}

/*
 * m_kick
 * parv[0] = sender prefix
 * parv[1] = channel
 * parv[2] = client to kick
 * parv[3] = kick comment
 */
gint m_kick(User *u, gint parc, gchar **parv)
{
    Channel *c;
    User *victim;
    
    g_return_val_if_fail(u != NULL, -1);

    c = _TBL(channel).get(parv[1]);
    g_return_val_if_fail(c != NULL, -1);

    victim = _TBL(user).get(parv[2]);
    g_return_val_if_fail(victim != NULL, -1);
    
    /* remove this user from the channel memberlist */
    remove_user_from_channel(c, u);
    /* remove this channel from the user chanlist */
    remove_channel_from_user_chanlist(c, u);
    
    return 0;
}

/*
 * m_motd 
 * parv[0] = sender prefix 
 * parv[1] = servername
 */
gint m_motd(User *u, gint parc, gchar **parv)
{
    gint i;
    gchar *motd[] = 
    {
	"MOTD: " SUX_VERSION,
	"This is the default MOTD.",
	"To edit it, RTFS.",
	NULL
    };

    for (i = 0; motd[i] != NULL; i++)
    {
	send_out(rpl_str(RPL_MOTD), me.name, parv[0], motd[i]);
    }
    for (i = 0; i < 20; i++)
    {	
	send_out(rpl_str(RPL_MOTD), me.name, parv[0],
		"SuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSux");
    }
    send_out(rpl_str(RPL_ENDOFMOTD), me.name, parv[0]);
    return 0;
}

/*
 *  m_server 
 *       parv[0] = sender prefix 
 *       parv[1] = servername 
 *       parv[2] = serverinfo/hopcount 
 *       parv[3] = serverinfo
 */
gint m_server(User *u, gint parc, gchar **parv)
{
    if(u == me.uplink)
    {
	gchar *name = parv[1];
	
	/* my uplink */
	if(me.uplink != _TBL(user).get(name))
	{
	    g_critical("No C/N Lines");

	    return 0;
	}

	if(mycmp(uplink.passwd, me.pass))
	{
	    g_critical("Access Denied (password mismatch)");

	    return 0;
	}

	if(!(uplink.capabs & NEEDED_CAPABS))
	{
	    g_critical("ERROR :Server does not support all the capabs I need");

	    return 0;
	}

	u = me.uplink;

	strcpy(u->info, parv[3]);
	
	if(uplink.ping_tag)
	{
	    g_source_del(uplink.ping_tag);
	    uplink.ping_tag = NULL;
	}

	g_timeout_source_add(10 seconds, (GSourceFunc) send_ping, u);

	uplink.firsttime = NOW;

	send_out(":%s GNOTICE :Link with %s[unknown@0.0.0.0] established, states: TS",
		me.name, u->name);
	send_out("BURST");

	send_nick_burst();
	
	uplink.flags |= FLAGS_SOBSENT|FLAGS_BURST;
	send_out("PING %s", me.name);
    }
    else
    {
	/* server behind my uplink */
	u = _TBL(user).get(parv[1]);
	g_return_val_if_fail(u == NULL, -1);

	u = _TBL(user).alloc(parv[1]);
	u->mode = atoi(parv[2]);
	strcpy(u->info, parv[3]);
    }

    return 0;
}

/*
 * m_info
 * parv[0] = sender prefix 
 * parv[1] = servername
 */
gint m_info(User *u, gint parc, gchar **parv)
{
    gint i;
    gchar *info[] = 
    {
	"INFO: " SUX_VERSION,
	"Coded and recoded by a nice coder =).",
	"mailto: vjt@azzurra.org",
	NULL
    };

    for (i = 0; info[i] != NULL; i++)
    {
	send_out(rpl_str(RPL_INFO), me.name, parv[0], info[i]);
    }

    for (i = 0; i < 20; i++)
    {
	send_out(rpl_str(RPL_INFO), me.name, parv[0],
		"SuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSuxSux");
    }

    send_out(rpl_str(RPL_ENDOFINFO), me.name, parv[0]);
    return 0;
}

/*
 * 
 * m_stats 
 * parv[0] = sender prefix 
 * parv[1] = statistics selector
 * parv[2] = server name (current server defaulted, if omitted) 
 */
gint m_stats(User *u, gint parc, gchar **parv)
{
    gchar stat = parc > 1 ? parv[1][0] : '*';
    
    switch(stat)
    {
	case 'C':
	case 'c':
	    send_out(rpl_str(RPL_STATSCLINE), me.name, parv[0],
		    SUX_UPLINK_NAME, SUX_UPLINK_PORT);
	    break;

	case 'M':
	case 'm':
	    send_message_count(parv[0]);
	    break;

	case 'U':
	case 'u':
	    {
		time_t now;
		
		now = NOW - me.boot;

		send_out(rpl_str(RPL_STATSUPTIME), me.name, parv[0],
			now / 86400, (now / 3600) % 24, (now / 60) % 60, now % 60);
	    }
	default:
	    break;
    }
    send_out(rpl_str(RPL_ENDOFSTATS), me.name, parv[0], stat);

    return 0;
}

gint m_version(User *u, gint parc, gchar **parv)
{
    send_out(rpl_str(RPL_VERSION),
	    me.name, parv[0], SUX_VERSION);
    return 0;
}

/*
 * m_squit
 * parv[0] = sender prefix 
 * parv[1] = server name 
 * parv[2] = comment
 */
gint m_squit(User *u, gint parc, gchar **parv)
{
    User *srv;

    g_return_val_if_fail(u != NULL, -1);
    
    srv = _TBL(user).get(parv[1]);
    g_return_val_if_fail(srv != NULL, -1);

    _TBL(user).del(srv);
    _TBL(user).destroy(srv);

    return 0;
}

/*
 * m_pass 
 * parv[0] = sender prefix 
 * parv[1] = password
 * parv[2] = optional extra version information
 */
gint m_pass(User *u, gint parc, gchar **parv)
{
    g_strlcpy(uplink.passwd, parv[1], sizeof(uplink.passwd));
    
    return 0;
}

/*
 * m_svinfo 
 *       parv[0] = sender prefix 
 *       parv[1] = TS_CURRENT for the server 
 *       parv[2] = TS_MIN for the server 
 *       parv[3] = server is standalone or connected to non-TS only 
 *       parv[4] = server's idea of UTC time
 */
gint m_svinfo(User *u, gint parc, gchar **parv)
{
    time_t deltat, theirtime, now;
    guint their_ts_ver, their_ts_min_ver;

    now = NOW;

    g_return_val_if_fail(parc > 4, -1);

    theirtime = atol(parv[4]);

    deltat = ABS(theirtime - now);

    if(deltat > 45)
    {
	g_critical("Link %s dropped, excessive TS delta (%ld)", u->name, deltat);

	return 0;
    }
    else if(deltat > 15)
    {
	g_warning("Link %s notable TS delta (my TS=%ld, their TS=%ld, delta=%ld",
		u->name, now, theirtime, deltat);
    }

    their_ts_ver = atoi(parv[1]);
    their_ts_min_ver = atoi(parv[2]);
    if(their_ts_ver < SUX_MIN_TS || SUX_CUR_TS < their_ts_min_ver)
    {
	g_critical_syslog("Incompatible TS version (%d,%d)",
		their_ts_ver, their_ts_min_ver);
	return 0;
    }

    me.uplink->ts = theirtime;

    return 0;
}

G_INLINE_FUNC gint cm_compare(ChanMember *cm, gchar *s)
{
    return mycmp(s, cm->u->name);
}

G_INLINE_FUNC gint ch_compare(SLink *lp, gchar *s)
{
    return mycmp(s, lp->value.c->chname);
}

static gint compile_mode(Mode *m, gchar *s, gint parc, gchar **parv)
{
    gint args = 0;

    memset(m, 0x0, sizeof(Mode));
    
    while(*s)
    {
	switch(*(s++))
	{
	    case 'i':
		m->mode |= MODE_INVITEONLY;
		break;
	    case 'n':
		m->mode |= MODE_NOPRIVMSGS;
		break;
	    case 'p':
		m->mode |= MODE_PRIVATE;
		break;
	    case 's':
		m->mode |= MODE_SECRET;
		break;
	    case 'm':
		m->mode |= MODE_MODERATED;
		break;
	    case 't':
		m->mode |= MODE_TOPICLIMIT;
		break;
	    case 'r':
		m->mode |= MODE_REGISTERED;
		break;
	    case 'R':
		m->mode |= MODE_REGONLY;
		break;
	    case 'M':
		m->mode |= MODE_MODREG;
		break;
	    case 'c':
		m->mode |= MODE_NOCOLOR;
		break;
	    case 'd':
		m->mode |= MODE_NONICKCHG;
		break;
	    case 'k':
		g_strlcpy(m->key, parv[4 + args], KEYLEN + 1);
		args++;
		if (parc < 5 + args)
		    return -1;
		break;
	    case 'l':
		m->limit = atoi(parv[4 + args]);
		args++;
		if (parc < 5 + args)
		    return -1;
		break;
	}
    }

    return args;
}

static void add_user_to_channel(User *u, Channel *c, guint flags)
{
    ChanMember *cm = g_mem_chunk_alloc0(_MPL(cmembers));
    SLink *lp;

    g_return_if_fail(c != NULL);
    g_return_if_fail(u != NULL);
    g_return_if_fail(cm != NULL);

    g_return_if_fail(g_slist_find_custom(u->channels, c->chname, (GCompareFunc) ch_compare) == NULL);
    
    cm->u = u;
    cm->flags = flags;

    lp = g_mem_chunk_alloc0(_MPL(links));
    lp->value.c = c;

    u->channels = g_slist_prepend(u->channels, lp);
    c->members = g_slist_prepend(c->members, cm);

}

/*
 * m_join 
 * parv[0] = sender prefix
 * parv[1] = channel
 * parv[2] = channel password (key)
 */
gint m_join(User *u, gint parc, gchar **parv)
{
    /* handle only `:user JOIN 0' case */
    GSList *johnny;
    g_return_val_if_fail(*parv[1] != '0', -1);

    johnny = u->channels;
    g_return_val_if_fail(u->channels != NULL, -1);

    while(johnny)
    {
	remove_user_from_channel(((SLink*)johnny->data)->value.c, u);
	johnny = g_slist_next(johnny);
    }

    return 0;
}

/*
 * m_sjoin 
 * parv[0] - sender 
 * parv[1] - TS 
 * parv[2] - channel 
 * parv[3] - modes + n arguments (key and/or limit) 
 * parv[4+n] - flags+nick list (all in one parameter)
 */
gint m_sjoin(User *u, gint parc, gchar **parv)
{
    gint ts;
    gchar *channel;
    gchar *users = NULL;
    gchar *modes;
    Channel *c = NULL;
    
    Mode mode;
    gint i, count;
    gchar *users_arr[512];
    gchar *s;

    g_return_val_if_fail(parv != NULL, 0);

    ts = atoi(parv[1]);
    channel = parv[2];

    if(!(c = _TBL(channel).get(channel)))
    {
	c = _TBL(channel).alloc(channel);
    }
    g_return_val_if_fail(c != NULL, 0);

    if(parc == 3)
    {
	User *u;
	
	/* client sjoin with timestamp */
	s = parv[0];
	c->ts = ts;

	u = _TBL(user).get(s);

	add_user_to_channel(_TBL(user).get(s), c, 0);

	return 0;
    }
    else
    {
	/* server sjoin with modes and users */

	modes = parv[3];

	if(*modes != '0')
	{
	    gint args = 0;
	    c->ts = ts;
	    c->bans = NULL;
	    c->members = NULL;

	    if((args = compile_mode(&mode, modes, parc, parv)) < 0)
	    {
		g_warning("Failed mode compilation on `%s', channel desynched !", modes);
		memset(&c->mode, 0x0, sizeof(Mode));
	    }
	    c->mode = mode;
	    users = parv[4 + args];
	}
	else
	{
	    users = parv[4];
	}

    }

    g_return_val_if_fail(users != NULL, -1);

    my_g_strsplit(users, ' ', 512, &count, users_arr);
    for(i = 0; i < count; i++)
    {
	gint fl = 0;
	
	s = users_arr[i];

	if(*s == '\0')
	    continue;

	if(*s == '@' || s[1] == '@')
	    fl |= MODE_CHANOP;

	if(*s == '+' || s[1] == '+')
	    fl |= MODE_VOICE;

	while(*s == '@' || *s == '+')
	    s++;
	
	if(!g_slist_find_custom(c->members, s, (GCompareFunc) cm_compare))
	{
	    add_user_to_channel(_TBL(user).get(s), c, fl);
	}
    }

    return 1;
}

gint m_capab(User *u, gint parc, gchar **parv)
{
    gint i;

    for(i = 1; i < parc; i++)
    {
	if (!mycmp(parv[i], "NOQUIT"))
	    uplink.capabs |= CAPAB_NOQUIT;
	else if (!mycmp(parv[i], "BURST"))
	    uplink.capabs |= CAPAB_BURST;
	else if (!mycmp(parv[i], "UNCONNECT"))
	    uplink.capabs |= CAPAB_UNCONN;
	else if (!mycmp(parv[i], "DKEY"))
	    uplink.capabs |= CAPAB_DKEY;
	else if (!mycmp(parv[i], "ZIP"))
	    uplink.capabs |= CAPAB_ZIP;
	else if (!mycmp(parv[i], "NICKIP"))
	    uplink.capabs |= CAPAB_NICKIP;
	else if (!mycmp(parv[i], "TSMODE"))
	    uplink.capabs |= CAPAB_TSMODE;
    }

    return 0;
}

gint m_lusers(User *u, gint parc, gchar **parv)
{
    g_return_val_if_fail(u != NULL, -1);
    
    send_out(rpl_str(RPL_LUSERCHANNELS),
	    me.name, u->name, _TBL(channel).count());
    
    send_out(rpl_str(RPL_GLOBALUSERS),
	    me.name, u->name, _TBL(user).count());

    return 0;
}

/*
 * m_whois 
 * parv[0] = sender prefix 
 * parv[1] = my name
 * parv[2] = nickname
 */
gint m_whois(User *u, gint parc, gchar **parv)
{
    User *target;
    gchar *nick = parv[2];
    gchar chanbuf[BUFSIZE];
    gint len, mlen;
    GSList *johnny; /* walker */
    Channel *c;

    g_return_val_if_fail(u != NULL, -1);

    target = _TBL(user).get(nick);

    if(target == NULL)
    {
	send_out(rpl_str(ERR_NOSUCHNICK),
		me.name, parv[0], nick);

	return 0;
    }

    send_out(rpl_str(RPL_WHOISUSER), me.name,
	    parv[0], target->name, target->username,
	    target->host, target->info);

    mlen = strlen(me.name) + strlen(parv[0]) + 6 + strlen(nick);
    for(len = 0, *chanbuf = '\0', johnny = target->channels;
	    johnny;
	    johnny = g_slist_next(johnny))
    {
	c = ((SLink*)johnny->data)->value.c;
	if(len + strlen(c->chname) > (gsize) BUFSIZE - 4 - mlen)
	{
	    send_out(":%s %d %s %s :%s",
		    me.name, RPL_WHOISCHANNELS,
		    parv[0], nick, chanbuf);
	    *chanbuf = '\0';
	    len = 0;
	}
	strcpy(chanbuf + len, c->chname);
	len += strlen(c->chname);
	chanbuf[len++] = ' ';
	chanbuf[len] = '\0';
    }
    if(chanbuf[0] != '\0')
    {
	send_out(rpl_str(RPL_WHOISCHANNELS),
		me.name, parv[0], nick, chanbuf);
    }

    send_out(rpl_str(RPL_WHOISSERVER),
	    me.name, parv[0], nick, target->server->name, target->server->info);

    send_out(rpl_str(RPL_ENDOFWHOIS),
	    me.name, parv[0], nick);

    return 0;
}

/* 
 * m_burst
 *      parv[0] = sender prefix
 *      parv[1] = SendQ if an EOB
 */
gint m_burst(User *u, gint parc, gchar **parv)
{
    g_return_val_if_fail(u != NULL, -1);

    if(parc == 2)
    {
	/* this is an EOB */
	time_t synch_time;
	
	synch_time = NOW - uplink.firsttime;

	uplink.flags &= ~FLAGS_EOBRECV;
	if(uplink.flags & (FLAGS_SOBSENT|FLAGS_BURST))
	    return 0;
	
	send_out(":%s GNOTICE :synch to %s in %ld sec%s at %s sendq",
		me.name, u->name, synch_time, synch_time == 1 ? "" : "s", parv[1]);
    }
    else
    {
	/* this is a SOB */
	uplink.flags |= FLAGS_EOBRECV;
    }

    return 0;
}

/*
 * m_away 
 * parv[0] = sender prefix 
 * parv[1] = away message
 */
gint m_away(User *u, gint parc, gchar **parv)
{
    gchar *msg = parv[1];
    
    g_return_val_if_fail(u != NULL, -1);
    
    if(parc < 2 || !*msg)
    {
	u->flags &= ~FLAGS_AWAY;
    }
    else
    {
	u->flags |= FLAGS_AWAY;
    }

    return 0;
}

gint m_cs(User *u, gint parc, gchar **parv)
{
    DUMMY
}
gint m_ns(User *u, gint parc, gchar **parv)
{
    DUMMY
}
gint m_ms(User *u, gint parc, gchar **parv)
{
    DUMMY
}
gint m_os(User *u, gint parc, gchar **parv)
{
    DUMMY
}
gint m_rs(User *u, gint parc, gchar **parv)
{
    DUMMY
}

gint m_time(User *u, gint parc, gchar **parv)
{
    gchar timebuf[25];
    
    ctime_r(&NOW, timebuf);
    
    send_out(rpl_str(RPL_TIME), me.name, parv[0], me.name, timebuf);

    return 0;
}

gint m_admin(User *u, gint parc, gchar **parv)
{
    send_out(rpl_str(RPL_ADMINME), me.name, parv[0], me.name);
    send_out(rpl_str(RPL_ADMINLOC1), me.name, parv[0], SUX_MODULE);
    send_out(rpl_str(RPL_ADMINLOC2), me.name, parv[0], "Release " SUX_RELEASE);
    send_out(rpl_str(RPL_ADMINEMAIL), me.name, parv[0], "coded by vjt@users.sf.net");

    return 0;
}
