#include "sux.h"
#include "h.h"
#include "main.h"
#include "table.h"
#include "numeric.h"
#include "log.h"
#include "match.h"

#define DUMMY return 0;

REMOTE_TABLE_INSTANCE(user);
REMOTE_TABLE_INSTANCE(channel);

REMOTE_MEMPOOL_INSTANCE(cmembers);
REMOTE_MEMPOOL_INSTANCE(links);

gint m_private(gint parc, gchar **parv)
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
gint m_topic(gint parc, gchar **parv)
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
gint m_join(gint parc, gchar **parv)
{
    DUMMY
}
gint m_mode(gint parc, gchar **parv)
{
    DUMMY
}
gint m_ping(gint parc, gchar **parv)
{
    send_out(":%s PONG :%s",
	    me.name, parv[1]);
    return 1;
}
gint m_pong(gint parc, gchar **parv)
{
    DUMMY
}
gint m_kick(gint parc, gchar **parv)
{
    DUMMY
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
gint m_nick(gint parc, gchar **parv)
{
    User *u;

    if(parc > 3)
    {
	/* new user. */

	u = _TBL(user).alloc(parv[1]);

	u->ts = strtoul(parv[3], NULL, 10);
	/* XXX: umode handling */
	strcpy(u->username, parv[5]);
	strcpy(u->host, parv[6]);
	strcpy(u->server, parv[7]);
	/* XXX: nickip handling */
	strcpy(u->gcos, parv[10]);

	return 1;
    }
    else
    {
	/* nick change */
	if((u = _TBL(user).get(parv[0])))
	{
	    if(!_TBL(user).del(u))
		abort();

	    strcpy(u->nick, parv[1]);
	    u->ts = strtoul(parv[2], NULL, 10);
	    _TBL(user).put(u);

	    return 1;
	}
	else
	{
	    send_out(":%s KILL %s :%s %s<-(?)", me.name, parv[1], me.name, parv[1]);
	    return 0;
	}
    }
}

gint m_error(gint parc, gchar **parv)
{
    g_critical_syslog("%s", parv[1]);

    return 0;
}
gint m_notice(gint parc, gchar **parv)
{
    DUMMY
}

/*
 * m_quit
 * parv[0] = sender prefix
 * parv[1] = comment
 */
gint m_quit(gint parc, gchar **parv)
{
    User *u = _TBL(user).get(parv[0]);
    Channel *c;
    GSList *johnny; /* the walker */

    g_return_val_if_fail(u != NULL, 0);

    for(johnny = u->channels; johnny; johnny = g_slist_next(u->channels))
    {
	GSList *jimmy; /* the second walker */
	c = _TBL(channel).get(((SLink *)johnny->data)->value.c->chname);
		
	/* XXX wtf does this not work ?? */
	for(jimmy = c->members; jimmy; jimmy = g_slist_next(c->members))
	{
	    if(((ChanMember*)jimmy->data)->u == u)
	    {
		/* remove this link */
		c->members = g_slist_remove_link(c->members, jimmy);
		g_mem_chunk_free(_MPL(cmembers), jimmy->data);
		g_slist_free_1(jimmy);

		break;
	    }
	}
    }

    for(johnny = u->channels; johnny; johnny = g_slist_next(u->channels))
    {
	g_mem_chunk_free(_MPL(links), johnny->data);
    }

    g_slist_free(u->channels);
    
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
gint m_part(gint parc, gchar **parv)
{
    gchar *pfx = parv[0];
    gchar *chname = parv[1];

    Channel *c;
    User *u;
    GSList *johnny; /* the walker */
    
    g_return_val_if_fail(pfx != NULL, 0);
    g_return_val_if_fail(chname != NULL, 0);

    c = _TBL(channel).get(chname);
    g_return_val_if_fail(c != NULL, 0);

    u = _TBL(user).get(pfx);
    g_return_val_if_fail(u != NULL, 0);
    
    for(johnny = c->members; johnny; johnny = g_slist_next(c->members))
    {
	if(((ChanMember*)johnny->data)->u == u)
	{
	    c->members = g_slist_remove_link(c->members, johnny);
	    g_mem_chunk_free(_MPL(cmembers), johnny->data);
	    g_slist_free_1(johnny);
	    
	    break;
	}
    }

    return 1;
}

gint m_kill(gint parc, gchar **parv)
{
    DUMMY
}
gint m_motd(gint parc, gchar **parv)
{
    gint i;
    gchar *motd[] = 
    {
	"MOTD: Sux Services ver 0.01.",
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
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
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
gint m_server(gint parc, gchar **parv)
{
    gint hopcount;
    
    strcpy(me.uplink, parv[1]);
    hopcount = atoi(parv[2]);

    g_message("Linked with %s [%s], distant %d hop%s",
	    me.uplink, parv[3], hopcount, hopcount != 1 ? "s" : "");

    return 0;
}

gint m_info(gint parc, gchar **parv)
{
    gint i;
    gchar *info[] = 
    {
	"INFO: Sux Services ver 0.01.",
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
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    }
    send_out(rpl_str(RPL_ENDOFINFO), me.name, parv[0]);
    return 0;
}

gint m_stats(gint parc, gchar **parv)
{
    DUMMY
}

gint m_version(gint parc, gchar **parv)
{
    send_out(rpl_str(RPL_VERSION),
	    me.name, parv[0], me.info);
    return 0;
}
gint m_squit(gint parc, gchar **parv)
{
    DUMMY
}
gint m_pass(gint parc, gchar **parv)
{
    DUMMY
}
gint m_umode(gint parc, gchar **parv)
{
    DUMMY
}

/*
 * m_svinfo 
 *       parv[0] = sender prefix 
 *       parv[1] = TS_CURRENT for the server 
 *       parv[2] = TS_MIN for the server 
 *       parv[3] = server is standalone or connected to non-TS only 
 *       parv[4] = server's idea of UTC time
 */
gint m_svinfo(gint parc, gchar **parv)
{
    time_t deltat, tmptime, theirtime;

    g_return_val_if_fail(parc > 4, 0);
    
    tmptime = time(NULL);
    theirtime = atol(parv[4]);
    deltat = abs(theirtime - tmptime);

    if(deltat > 45)
    {
	g_warning("Link %s dropped, excessive TS delta (%ld)", parv[0], deltat);
	send_out(":%s SQUIT %s :excessive TS delta (%ld)",
		me.name, parv[0], deltat);
	STOP_RUNNING();
    }
    else if(deltat > 15)
    {
	g_warning("Link %s notable TS delta (my TS=%ld, their TS=%ld, delta=%ld",
		parv[0], tmptime, theirtime, deltat);
    }

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

static gboolean compile_mode(Mode *m, gchar *s, gint parc, gchar **parv)
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
		strncpy(m->key, parv[4 + args], KEYLEN + 1);
		args++;
		if (parc < 5 + args)
		    return FALSE;
		break;
	    case 'l':
		m->limit = atoi(parv[4 + args]);
		args++;
		if (parc < 5 + args)
		    return FALSE;
		break;
	}
    }

    return TRUE;
}

static void add_user_to_channel(User *u, Channel *c, guint flags)
{
    ChanMember *cm = g_mem_chunk_alloc0(_MPL(cmembers));

    g_return_if_fail(c != NULL);
    g_return_if_fail(u != NULL);
    g_return_if_fail(cm != NULL);
    
    cm->u = u;
    cm->flags = flags;

    if(!g_slist_find_custom(u->channels, c->chname, (GCompareFunc) ch_compare))
    {
	SLink *lp = g_mem_chunk_alloc0(_MPL(links));
	lp->value.c = c;
	u->channels = g_slist_prepend(u->channels, lp);
    }	
    
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
gint m_sjoin(gint parc, gchar **parv)
{
    gint ts;
    gchar *channel;
    gchar *users;
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

	modes = parv[3];
	users = parv[4];
	
	c->ts = ts;
	c->bans = NULL;
	c->members = NULL;
	if(compile_mode(&mode, modes, parc, parv) == FALSE)
	{
	    g_warning("failed mode compilation on %s", modes);
	    memset(&c->mode, 0x0, sizeof(Mode));
	    return 0;
	}
	c->mode = mode;
    }

    users_arr = g_strsplit(users, " ", 0);
    for(i = 0; users_arr[i] != NULL; i++)
    {
	gint fl;
	
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

gint m_capab(gint parc, gchar **parv)
{
    DUMMY
}
gint m_burst(gint parc, gchar **parv)
{
    DUMMY
}
gint m_away(gint parc, gchar **parv)
{
    DUMMY
}
gint m_nickcoll(gint parc, gchar **parv)
{
    DUMMY
}
gint m_cs(gint parc, gchar **parv)
{
    DUMMY
}
gint m_ns(gint parc, gchar **parv)
{
    DUMMY
}
gint m_ms(gint parc, gchar **parv)
{
    DUMMY
}
gint m_os(gint parc, gchar **parv)
{
    DUMMY
}
gint m_rs(gint parc, gchar **parv)
{
    DUMMY
}
gint m_time(gint parc, gchar **parv)
{
    DUMMY
}
gint m_admin(gint parc, gchar **parv)
{
    DUMMY
}
gint m_gnotice(gint parc, gchar **parv)
{
    DUMMY
}
