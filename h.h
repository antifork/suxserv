#ifndef __h_h__
#define __h_h__

extern int  m_private(int, char **);
extern int  m_topic(int, char **);
extern int  m_join(int, char **);
extern int  m_part(int, char **);
extern int  m_mode(int, char **);
extern int  m_ping(int, char **);
extern int  m_pong(int, char **);
extern int  m_kick(int, char **);
extern int  m_nick(int, char **);
extern int  m_error(int, char **);
extern int  m_notice(int, char **);
extern int  m_quit(int, char **);
extern int  m_kill(int, char **);
extern int  m_server(int, char **);
extern int  m_info(int, char **);
extern int  m_stats(int, char **);
extern int  m_version(int, char **);
extern int  m_squit(int, char **);
extern int  m_pass(int, char **);
extern int  m_umode(int, char **);
extern int  m_motd(int, char **);
extern int  m_svinfo(int, char **);
extern int  m_sjoin(int, char **);
extern int  m_time(int, char **);
extern int  m_capab(int, char **);
extern int  m_burst(int, char **);
extern int  m_away(int, char **);
extern int  m_admin(int, char **);
extern int  m_gnotice(int, char **);
extern int  m_nickcoll(int, char **);
extern int  m_cs(int, char **);
extern int  m_ns(int, char **);
extern int  m_os(int, char **);
extern int  m_rs(int, char **);
extern int  m_ms(int, char **);

extern void send_out();

#endif /* __h_h__ */
