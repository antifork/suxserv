#include "sux.h"
#include "h.h"
#include "main.h"
#include "usertable.h"
#include "numeric.h"

#define DUMMY return 0;


gint m_private(gint parc, gchar **parv)
{
    DUMMY
}
gint m_topic(gint parc, gchar **parv)
{
    DUMMY
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
    if(parc > 3)
    {
	User *u;
	/* new user. */
	u = usertable.alloc(&usertable, parv[1]);
	strcpy(u->nick, parv[1]);
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
	User u;
	
	/* nick change */
	if(usertable.get(&usertable, parv[0], &u)) /* XXX: implement get and delete for TABLE_T */
	{
	    if(!usertable.del(&usertable, parv[0]))
		exit(1);

	    strcpy(u.nick, parv[1]);
	    u.ts = strtoul(parv[2], NULL, 10);
	    usertable.put(&usertable, parv[1], &u);

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
    errno = 0;
    fatal("%s", parv[1]);
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
    usertable.del(&usertable, parv[0]);
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
    fatal("received SQUIT for %s [%s]",
	    parv[1], parv[2]);
    return 0;
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
gint m_sjoin(gint parc, gchar **parv)
{
    DUMMY
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
