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
 *    This product includes software developed by Chip Norkus.
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
extern gint  m_kill(User *, gint, gchar **);
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
extern gint  m_lusers(User *, gint, gchar **);
extern gint  m_whois(User *, gint, gchar **);
extern gint  m_cs(User *, gint, gchar **);
extern gint  m_ns(User *, gint, gchar **);
extern gint  m_os(User *, gint, gchar **);
extern gint  m_rs(User *, gint, gchar **);
extern gint  m_ms(User *, gint, gchar **);
extern void  nego_start(void);

#endif /* __h_h__ */
