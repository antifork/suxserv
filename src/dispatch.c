#include "sux.h"
#include "h.h"
#include "main.h"
#include "table.h"
#include "numeric.h"
#include "log.h"
#include "match.h"
#include "network.h"
#include "parse.h"

#define DUMMY return 0;


REMOTE_TABLE_INSTANCE(user);
REMOTE_TABLE_INSTANCE(channel);

REMOTE_MEMPOOL_INSTANCE(cmembers);
REMOTE_MEMPOOL_INSTANCE(links);

static struct
{
    gshort capabs;
    gchar passwd[PASSWDLEN + 1];
    gchar name[PASSWDLEN + 1];
    User *ptr;

    guint ping_tag;
    time_t firsttime;
    gshort flags;
    
} uplink;

static gboolean ping_timeout(User *u)
{
    /* ping timeout */
    g_critical("Ping Timeout (%d usecs)", PING_FREQUENCY);
    
    return FALSE;
}

static gboolean send_ping(User *u)
{
    if(uplink.ping_tag != -1)
    {
	return ping_timeout(u);
    }

    uplink.ping_tag = g_timeout_add(PING_TIMEOUT, (GSourceFunc) ping_timeout, u);

    send_out("PING :%s", u->nick);

    return TRUE;
}
    
void nego_start(void)
{
    send_out("PASS %s :TS", me.pass);
    send_out("CAPAB BURST NOQUIT SSJOIN UNCONNECT NICKIP TSMODE");
    send_out("SVINFO 5 3 0 :%lu", time(NULL));
    send_out("SERVER %s 1 :%s", me.name, me.info);

    uplink.ping_tag = g_timeout_add(PING_TIMEOUT, (GSourceFunc) ping_timeout, NULL);
    uplink.ptr = _TBL(user).alloc(SUX_UPLINK_NAME);
}

gint m_private(User *u, gint parc, gchar **parv)
{
    DUMMY
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
    if(uplink.flags & (FLAGS_SOBSENT|~FLAGS_BURST))
    {
	uplink.flags &= ~FLAGS_SOBSENT;
	send_out("BURST %d",
		g_io_channel_get_buffer_condition(me.handle) & G_IO_OUT ?
		me.handle->write_buf->len : 0);
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

    if(uplink.ping_tag != -1)
    {
	g_return_val_if_fail(g_source_remove(uplink.ping_tag) == TRUE, -1);
	uplink.ping_tag = -1;
	return 0;
    }

    if(u != uplink.ptr)
    {
	return 0;
    }

    if(uplink.flags & FLAGS_USERBURST)
    {
	uplink.flags &= ~FLAGS_USERBURST;
	send_out(":%s GNOTICE :%s has processed user/channel burst, "
		"sending topic burst.", me.name, u->nick);

	uplink.flags |= FLAGS_SOBSENT;
	/* XXX: send topic burst */
	send_out("PING :%s", me.name);
    }
    else if(uplink.flags & FLAGS_TOPICBURST)
    {
	uplink.flags &= ~FLAGS_TOPICBURST;
	send_out(":%s GNOTICE :%s has processed topic burst (synched "
		"to network data).", me.name, u->nick);
	send_out("PING :%s", me.name);
    }

    return 0;
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
	    g_error("new user %s already exists in hash table ..",
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
	strcpy(u->gcos, parv[10]);
    }
    else
    {
	/* nick change */
	g_return_val_if_fail(u != NULL, -1);

	if(!_TBL(user).del(u))
	    g_error("cannot delete user");

	strcpy(u->nick, parv[1]);
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

gint m_notice(User *u, gint parc, gchar **parv)
{
    DUMMY
}

static void list_free_atoms(gpointer *data, GMemChunk *chunk)
{
    g_return_if_fail(data != NULL);
    g_return_if_fail(chunk != NULL);

    g_mem_chunk_free(chunk, data);
}

static gboolean remove_user_from_channel(Channel *c, User *u)
{
    register GSList *johnny; /* the walker */
    register ChanMember *cm;

    for(johnny = c->members; johnny; johnny = g_slist_next(johnny))
    {
	g_return_val_if_fail(johnny->data != NULL, FALSE);

	cm = (ChanMember *) johnny->data;
	g_return_val_if_fail(cm->u != NULL, 0);

	if(cm->u == u)
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

	    g_return_val_if_fail(remove_user_from_channel(c, u) == TRUE, -1);
	}

	g_slist_foreach(u->channels, (GFunc) list_free_atoms, _MPL(links));
	g_slist_free(u->channels);
    }
    
    _TBL(user).del(u);
    _TBL(user).destroy(u);
    return 1;
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
    g_return_val_if_fail(remove_user_from_channel(c, u) == TRUE, -1);
    /* remove this channel from the user chanlist */
    g_return_val_if_fail(remove_channel_from_user_chanlist(c, u) == TRUE, -1);

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
    g_return_val_if_fail(remove_user_from_channel(c, u) == TRUE, -1);
    /* remove this channel from the user chanlist */
    g_return_val_if_fail(remove_channel_from_user_chanlist(c, u) == TRUE, -1);
    
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
    if(u == NULL)
    {
	gchar *name = parv[1];
	
	/* my uplink */
	if(uplink.ptr != _TBL(user).get(name))
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

	u = me.uplink = uplink.ptr;

	strcpy(u->gcos, parv[3]);
	
	g_source_remove(uplink.ping_tag);
	uplink.ping_tag = -1;
	g_timeout_add(PING_FREQUENCY, (GSourceFunc) send_ping, u);

	uplink.firsttime = time(NULL);
	send_out(":%s GNOTICE :Link with %s[%s] established, states: TS",
		me.name, u->nick, SUX_UPLINK_HOST);
	
	send_out("BURST");
	/* XXX: send our agents here */
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
	strcpy(u->gcos, parv[3]);
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
    g_return_val_if_fail(me.uplink == NULL, -1);

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
    time_t deltat, tmptime, theirtime;
    guint their_ts_ver, their_ts_min_ver;

    g_return_val_if_fail(parc > 4, -1);

    tmptime = time(NULL);
    theirtime = atol(parv[4]);
    deltat = ABS(theirtime - tmptime);

    if(deltat > 45)
    {
	g_critical("Link %s dropped, excessive TS delta (%ld)", u->nick, deltat);

	return 0;
    }
    else if(deltat > 15)
    {
	g_warning("Link %s notable TS delta (my TS=%ld, their TS=%ld, delta=%ld",
		u->nick, tmptime, theirtime, deltat);
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
    return mycmp(s, cm->u->nick);
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
    gchar **users_arr;
    gchar *s;
    Channel *c = NULL;
    Mode mode;
    gint i;

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
	/* client sjoin with timestamp */
	s = parv[0];
	c->ts = ts;

	add_user_to_channel(_TBL(user).get(s), c, 0);

	return 0;
    }
    else
    {
	/* server sjoin with modes and users */
	gint args;

	modes = parv[3];
	
	c->ts = ts;
	c->bans = NULL;
	c->members = NULL;
	if((args = compile_mode(&mode, modes, parc, parv)) < 0)
	{
	    g_warning("failed mode compilation on %s", modes);
	    memset(&c->mode, 0x0, sizeof(Mode));
	    return 0;
	}
	c->mode = mode;
	users = parv[4 + args];
    }

    g_return_val_if_fail(users != NULL, -1);

    users_arr = g_strsplit(users, " ", 0);
    for(i = 0; users_arr[i] != NULL; i++)
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
    
    g_strfreev(users_arr);

    return 1;
}

gint m_capab(User *u, gint parc, gchar **parv)
{
    gint i;

    g_return_val_if_fail(me.uplink == NULL, -1);

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
	    me.name, u->nick, _TBL(channel).count());
    
    send_out(rpl_str(RPL_GLOBALUSERS),
	    me.name, u->nick, _TBL(user).count());

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
	    parv[0], target->nick, target->username,
	    target->host, target->gcos);

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
	    me.name, parv[0], nick, target->server->nick, target->server->gcos);

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
	time_t synch_time = time(NULL) - uplink.firsttime;

	uplink.flags &= ~FLAGS_EOBRECV;
	if(uplink.flags & (FLAGS_SOBSENT|FLAGS_BURST))
	    return 0;
	
	send_out(":%s GNOTICE :synch to %s in %ld sec%s at %s sendq",
		me.name, u->nick, synch_time, synch_time == 1 ? "" : "s", parv[1]);
    }
    else
    {
	/* this is a SOB */
	uplink.flags |= FLAGS_EOBRECV;
    }

    return 0;
}

gint m_away(User *u, gint parc, gchar **parv)
{
    DUMMY
}
gint m_nickcoll(User *u, gint parc, gchar **parv)
{
    DUMMY
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
    DUMMY
}
gint m_admin(User *u, gint parc, gchar **parv)
{
    DUMMY
}
gint m_gnotice(User *u, gint parc, gchar **parv)
{
    DUMMY
}
