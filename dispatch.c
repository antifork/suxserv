#include "sux.h"
#include "h.h"
#include "main.h"
#include "usertable.h"
#include "numeric.h"

#define DUMMY return 0;

int m_private(int parc, char **parv)
{
    DUMMY
}
int m_topic(int parc, char **parv)
{
    DUMMY
}
int m_join(int parc, char **parv)
{
    DUMMY
}
int m_part(int parc, char **parv)
{
    DUMMY
}
int m_mode(int parc, char **parv)
{
    DUMMY
}
int m_ping(int parc, char **parv)
{
    send_out(":%s PONG :%s",
	    me.name, parv[1]);
    return 1;
}
int m_pong(int parc, char **parv)
{
    DUMMY
}
int m_kick(int parc, char **parv)
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
int m_nick(int parc, char **parv)
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

int m_error(int parc, char **parv)
{
    extern void fatal(char *, ...);
    errno = 0;
    fatal(parv[1]);
    return 0;
}
int m_notice(int parc, char **parv)
{
    DUMMY
}

/*
 * m_quit
 * parv[0] = sender prefix
 * parv[1] = comment
 */
int m_quit(int parc, char **parv)
{
    usertable.del(&usertable, parv[0]);
    return 1;
}
int m_kill(int parc, char **parv)
{
    DUMMY
}
int m_motd(int parc, char **parv)
{
    int i;
    char *motd[] = 
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
    for (i = 0; i < 200; i++)
    {
	send_out(rpl_str(RPL_MOTD), me.name, parv[0], "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    }
    send_out(rpl_str(RPL_ENDOFMOTD), me.name, parv[0]);
    return 0;
}
int m_server(int parc, char **parv)
{
    DUMMY
}
int m_info(int parc, char **parv)
{
    int i;
    char *info[] = 
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
    for (i = 0; i < 200; i++)
    {
	send_out(rpl_str(RPL_INFO), me.name, parv[0], "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    }
    send_out(rpl_str(RPL_ENDOFINFO), me.name, parv[0]);
    return 0;
}
int m_stats(int parc, char **parv)
{
    DUMMY
}
int m_version(int parc, char **parv)
{
    send_out(rpl_str(RPL_VERSION),
	    me.name, parv[0], me.info);
    return 0;
}
int m_squit(int parc, char **parv)
{
    DUMMY
}
int m_pass(int parc, char **parv)
{
    DUMMY
}
int m_umode(int parc, char **parv)
{
    DUMMY
}
int m_svinfo(int parc, char **parv)
{
    DUMMY
}
int m_sjoin(int parc, char **parv)
{
    DUMMY
}
int m_capab(int parc, char **parv)
{
    DUMMY
}
int m_burst(int parc, char **parv)
{
    DUMMY
}
int m_away(int parc, char **parv)
{
    DUMMY
}
int m_nickcoll(int parc, char **parv)
{
    DUMMY
}
int m_cs(int parc, char **parv)
{
    DUMMY
}
int m_ns(int parc, char **parv)
{
    DUMMY
}
int m_ms(int parc, char **parv)
{
    DUMMY
}
int m_os(int parc, char **parv)
{
    DUMMY
}
int m_rs(int parc, char **parv)
{
    DUMMY
}
int m_time(int parc, char **parv)
{
    DUMMY
}
int m_admin(int parc, char **parv)
{
    DUMMY
}
int m_gnotice(int parc, char **parv)
{
    DUMMY
}
