#include "services.h"
#include "h.h"
#include "main.h"
#include "table.h"

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
    User *debug;
    if(parc > 2)
    {
	User *u;
	/* new user. */
	u = usertable.alloc(&usertable, parv[1]);
	strcpy(u->nick, parv[1]);
	u->ts = strtol(parv[3], NULL, 10);
	/* XXX: umode handling */
	strcpy(u->username, parv[5]);
	strcpy(u->host, parv[6]);
	strcpy(u->server, parv[7]);
	/* XXX: nickip handling */
	strcpy(u->gcos, parv[10]);

	debug = usertable.getp(&usertable, parv[1]);
	fprintf(stderr, "allocated new nick: %s!%s@%s on %s\n", 
		debug->nick, debug->username, debug->host, debug->server);
	return 1;
    }
    else
    {
	User u;
	
	char oldnick[NICKLEN];
	
	/* nick change */
	usertable.get(&usertable, parv[0], &u); /* XXX: implement get and delete for TABLE_T */
	usertable.del(&usertable, parv[0]);
	
	strcpy(oldnick, u.nick);
	
	strcpy(u.nick, parv[1]);
	usertable.put(&usertable, parv[1], &u);

	debug = usertable.getp(&usertable, parv[1]);
	fprintf(stderr, "nickchange: %s -> %s\n", 
		oldnick, debug->nick);
	return 1;
    }
    return 1;
}

int m_error(int parc, char **parv)
{
    DUMMY
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
}
int m_kill(int parc, char **parv)
{
    DUMMY
}
int m_motd(int parc, char **parv)
{
    int i;
    for (i = 0; i < 40; i++)
    {
	send_out(":%s NOTICE %s : dio porcone animale carogna buco di culo shiatto",
		me.name, parv[0]);
    }
    return 0;
}
int m_server(int parc, char **parv)
{
    DUMMY
}
int m_info(int parc, char **parv)
{
    DUMMY
}
int m_stats(int parc, char **parv)
{
    DUMMY
}
int m_version(int parc, char **parv)
{
    send_out(":%s NOTICE %s :%s",
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
