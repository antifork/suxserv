#ifndef __h_h__
#define __h_h__

#include "sux.h"

extern gint  m_private(User *, gint, gchar **);
extern gint  m_topic(User *, gint, gchar **);
extern gint  m_part(User *, gint, gchar **);
extern gint  m_mode(User *, gint, gchar **);
extern gint  m_ping(User *, gint, gchar **);
extern gint  m_pong(User *, gint, gchar **);
extern gint  m_kick(User *, gint, gchar **);
extern gint  m_nick(User *, gint, gchar **);
extern gint  m_error(User *, gint, gchar **);
extern gint  m_notice(User *, gint, gchar **);
extern gint  m_quit(User *, gint, gchar **);
extern gint  m_server(User *, gint, gchar **);
extern gint  m_info(User *, gint, gchar **);
extern gint  m_stats(User *, gint, gchar **);
extern gint  m_version(User *, gint, gchar **);
extern gint  m_squit(User *, gint, gchar **);
extern gint  m_pass(User *, gint, gchar **);
extern gint  m_motd(User *, gint, gchar **);
extern gint  m_svinfo(User *, gint, gchar **);
extern gint  m_sjoin(User *, gint, gchar **);
extern gint  m_time(User *, gint, gchar **);
extern gint  m_capab(User *, gint, gchar **);
extern gint  m_burst(User *, gint, gchar **);
extern gint  m_away(User *, gint, gchar **);
extern gint  m_admin(User *, gint, gchar **);
extern gint  m_gnotice(User *, gint, gchar **);
extern gint  m_nickcoll(User *, gint, gchar **);
extern gint  m_cs(User *, gint, gchar **);
extern gint  m_ns(User *, gint, gchar **);
extern gint  m_os(User *, gint, gchar **);
extern gint  m_rs(User *, gint, gchar **);
extern gint  m_ms(User *, gint, gchar **);
extern void  nego_start(void);

#endif /* __h_h__ */
