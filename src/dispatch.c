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
gint m_part(gint parc, gchar **parv)
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
    
    _TBL(user).del(u);
    _TBL(user).destroy(u);
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
	send_out(rpl_str(RPL_MOTD), me.name, parv[0], "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    }
    send_out(rpl_str(RPL_ENDOFMOTD), me.name, parv[0]);
    return 0;
}
gint m_server(gint parc, gchar **parv)
{
    DUMMY
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
	send_out(rpl_str(RPL_INFO), me.name, parv[0], "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
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
gint m_svinfo(gint parc, gchar **parv)
{
    DUMMY
}

static gint cm_compare(ChanMember *cm, gchar *s)
{
    return mycmp(s, cm->u->nick);
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
    gint args = 0;
    gchar *channel;
    gchar *users;
    gchar *modes;
    gchar **users_arr;
    gchar *s;
    Channel *c = NULL;
    Mode mode;
    gint i;
    
    if(parc < 5)
    {
	gchar buf[512];

	memset(buf, 0x0, sizeof(buf));
	for(i = 0; i < parc; i++)
	    strncat(buf, parv[i], sizeof(buf));
	    
	g_warning("received SJOIN with less than 5 params (%d): %s", parc, buf);
	return 0;
    }
    
    ts = atoi(parv[1]);
    channel = parv[2];
    modes = parv[3];
    users = parv[4];

    if(!(c = _TBL(channel).get(channel)))
    {
	c = _TBL(channel).alloc(channel);
	c->ts = ts;
	c->bans = NULL;
	c->users = NULL;
    }
    else if(c->ts != ts)
    {
	c->ts = ts;
    }

    memset(&mode, 0x0, sizeof(mode));
    
    while(*modes)
    {
	switch(*(modes++))
	{
	    case 'i':
		mode.mode |= MODE_INVITEONLY;
		break;
	    case 'n':
		mode.mode |= MODE_NOPRIVMSGS;
		break;
	    case 'p':
		mode.mode |= MODE_PRIVATE;
		break;
	    case 's':
		mode.mode |= MODE_SECRET;
		break;
	    case 'm':
		mode.mode |= MODE_MODERATED;
		break;
	    case 't':
		mode.mode |= MODE_TOPICLIMIT;
		break;
	    case 'r':
		mode.mode |= MODE_REGISTERED;
		break;
	    case 'R':
		mode.mode |= MODE_REGONLY;
		break;
	    case 'M':
		mode.mode |= MODE_MODREG;
		break;
	    case 'c':
		mode.mode |= MODE_NOCOLOR;
		break;
	    case 'd':
		mode.mode |= MODE_NONICKCHG;
		break;
	    case 'k':
		strncpy(mode.key, parv[4 + args], KEYLEN + 1);
		args++;
		if (parc < 5 + args)
		    return 0;
		break;
	    case 'l':
		mode.limit = atoi(parv[4 + args]);
		args++;
		if (parc < 5 + args)
		    return 0;
		break;
	}
    }
    c->mode = mode;

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
	
	if(!g_slist_find_custom(c->users, s, (GCompareFunc) cm_compare))
	{
	    ChanMember *cm = g_mem_chunk_alloc0(_MPL(cmembers));
	    cm->u = _TBL(user).get(s);
	    cm->flags = fl;
	    
	    c->users = g_slist_prepend(c->users, cm);
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
