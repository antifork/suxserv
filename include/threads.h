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

#ifndef __threads_h__
#define __threads_h__

extern void setup_mutexes(void);
extern void spawn_threads(void);
extern void wait_for_termination(void);
extern void clean_exit(void);
extern void push_signal(gint *signum);
extern void start_master_thread(void);

extern G_INLINE_FUNC void g_source_del(GSource *);
extern G_INLINE_FUNC GSource *g_timeout_source_add(guint, GSourceFunc, gpointer);
extern G_INLINE_FUNC GSource *g_source_add(GIOChannel *, GIOCondition, GIOFunc);

#define GLOBAL_RUN_DECLARE()	static gboolean __is_running; static GMutex *__run_mutex
#define GLOBAL_RUN_INIT()	__is_running = TRUE; __run_mutex = g_mutex_new()

#define __RUN_LOCK()		g_mutex_lock(__run_mutex)
#define __RUN_UNLOCK()		g_mutex_unlock(__run_mutex)

#define START_RUNNING()		__RUN_LOCK(); __is_running = TRUE; __RUN_UNLOCK()
#define STOP_RUNNING()		__RUN_LOCK(); __is_running = FALSE; __RUN_UNLOCK()

#define THREAD_RUN_DECLARE()	gboolean __is_thread_running
#define THREAD_IS_RUNNING()	(__is_thread_running)

#define THREAD_RUN_CHECK()	__RUN_LOCK(); __is_thread_running = __is_running; __RUN_UNLOCK()

#endif
