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


#include "sux.h"
#include "main.h"
#include "network.h"
#include "log.h"
#include "threads.h"

GIOChannel *connect_server(gchar *host, guint port)
{
    struct sockaddr_in sock, my_addr;
    gint fd;
    socklen_t namelen = sizeof(my_addr);
    GIOChannel *ret;

    gchar hostip[16]; /* abc.def.ghi.jkl0
    			 1234567890123456*/

    g_return_val_if_fail(host != NULL, NULL);
    g_return_val_if_fail(port > 0, NULL);

    if(!inet_pton(AF_INET, host, (void*)&sock.sin_addr))
    {
	struct hostent *he = gethostbyname(host);
	if(!he)
	    g_errno_critical("gethostbyname()");
	memcpy(&sock.sin_addr, he->h_addr_list[0], he->h_length);
    }

    sock.sin_port = htons(port);
    sock.sin_family = AF_INET;

    inet_ntop(AF_INET, (const void *) &sock.sin_addr, hostip, sizeof(hostip));

    g_message_syslog("Connecting to %s[%s] ... ", host, hostip);

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	g_errno_critical("socket()");

    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = 0;
    my_addr.sin_family = AF_INET;

    if(bind(fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) < 0)
	g_errno_critical("bind()");

    if(connect(fd, (struct sockaddr *)&sock, sizeof(struct sockaddr_in)) < 0)
	g_errno_critical("connect()");

    if(getsockname(fd, (struct sockaddr *)&my_addr, &namelen) < 0)
	g_errno_critical("getsockname()");

    g_message_syslog("Connection established");

    if((ret = g_io_channel_unix_new(fd)))
    {
	GError *err = NULL;
	
	g_io_channel_set_encoding(ret, NULL, &err);

	if(err)
	{
	    g_critical_syslog(err->message);
	    g_error_free(err);

	    g_io_channel_shutdown(ret, FALSE, NULL);
	    g_io_channel_unref(ret);

	    return NULL;
	}

	g_io_channel_set_flags(ret, G_IO_FLAG_NONBLOCK, &err);

	if(err)
	{
	    g_critical_syslog(err->message);
	    g_error_free(err);

	    g_io_channel_shutdown(ret, FALSE, NULL);
	    g_io_channel_unref(ret);

	    return NULL;
	}

	g_io_channel_set_buffered(ret, FALSE);
    }

    return(ret);
}

gboolean net_shutdown(GIOChannel *source)
{
    GError *err = NULL;
    while(TRUE)
    {
	switch(g_io_channel_shutdown(source, TRUE, &err))
	{
	    case G_IO_STATUS_ERROR:
		g_critical_syslog("Close error: %s", err->message);
		g_error_free(err);
		return FALSE;
	    case G_IO_STATUS_AGAIN:
		continue;
	    case G_IO_STATUS_NORMAL:
	    case G_IO_STATUS_EOF:
		return TRUE;
	}
    }
    
    return FALSE;
}

static GString *net_w_buf;
static gchar *net_r_buf;
static gchar *send_out_buf;
void setup_netbuf(void)
{
    me.sendQ = g_string_sized_new(WRITEBUFSZ);
    me.recvQ = g_string_sized_new(READBUFSZ);

    net_w_buf = g_string_sized_new(WRITEBUFSZ);
    net_r_buf = g_malloc0(READBUFSZ);

    send_out_buf = g_malloc0(BUFSIZE);
}

gboolean net_receive_callback(GIOChannel *handle)
{
    GError *err = NULL;
    gsize bytes_read;

    g_return_val_if_fail(handle != NULL, FALSE);

    while(TRUE)
    {
	switch(g_io_channel_read_chars(handle, net_r_buf, READBUFSZ, &bytes_read, &err))
	{
	    case G_IO_STATUS_NORMAL:
		g_mutex_lock(me.readbuf_mutex);
		net_r_buf[bytes_read] = '\0';
		me.recvQ = g_string_append(me.recvQ, net_r_buf);
		g_cond_signal(me.readbuf_cond);
		g_mutex_unlock(me.readbuf_mutex);

		return TRUE;
	    case G_IO_STATUS_ERROR:
		g_mutex_lock(me.tag_mutex);
		g_source_del(me.recv_tag);
		me.recv_tag = NULL;
		g_mutex_unlock(me.tag_mutex);

		g_critical_syslog("Read error: %s", err->message);
		g_error_free(err);
		return FALSE;
    
	    case G_IO_STATUS_EOF:
		g_mutex_lock(me.tag_mutex);
		g_source_del(me.recv_tag);
		me.recv_tag = NULL;
		g_mutex_unlock(me.tag_mutex);

		g_critical_syslog("Read error: Connection closed");
		return FALSE;

	    case G_IO_STATUS_AGAIN:
		return TRUE;

	    default:
		g_error("Unknown status returned by net_receive() [this is a bug !]");
		return FALSE;
	}
    }
    
    abort();
    return FALSE;
}

gboolean net_send_callback(GIOChannel *dest)
{
    GError *err = NULL;
    gsize bytes_written;
    gboolean tried = FALSE;

    g_return_val_if_fail(dest != NULL, FALSE);

    g_mutex_lock(me.writebuf_mutex);
    net_w_buf = g_string_append(net_w_buf, me.sendQ->str);
    me.sendQ = g_string_erase(me.sendQ, 0, -1);
    g_mutex_unlock(me.writebuf_mutex);

    while(TRUE)
    {
	switch(g_io_channel_write_chars(dest, net_w_buf->str, net_w_buf->len, &bytes_written, &err))
	{
	    case G_IO_STATUS_ERROR:
		g_mutex_lock(me.tag_mutex);
		g_source_del(me.send_tag);
		me.send_tag = NULL;
		g_mutex_unlock(me.tag_mutex);

		g_critical_syslog("Write error: %s", err->message);
		g_error_free(err);
		return FALSE;

	    case G_IO_STATUS_NORMAL:
		net_w_buf = g_string_erase(net_w_buf, 0, bytes_written);
		if(!tried && net_w_buf->len)
		{
		    /* try once to deliver remaining data */
		    tried = TRUE;
		    continue;
		}

		g_mutex_lock(me.tag_mutex);
		if(!net_w_buf->len && me.send_tag)
		{
		    g_source_del(me.send_tag);
		    me.send_tag = NULL;
		}
		g_mutex_unlock(me.tag_mutex);

		/* fallthrough */
	    case G_IO_STATUS_AGAIN:
	    case G_IO_STATUS_EOF:
		return TRUE;
	}
    }
    g_error_free(err);

    abort();
    return FALSE;
}

gboolean net_err_callback(GIOChannel *dest)
{
    g_return_val_if_fail(dest != NULL, FALSE);

    g_critical_syslog("Network error");

    return FALSE;
}

void send_out(gchar *fmt, ...)
{
    va_list ap;
    gsize len;

    va_start(ap, fmt);
    len = g_vsnprintf(send_out_buf, BUFSIZE, fmt, ap);
    va_end(ap);

    if(len > BUFSIZE - 2)
    {
	send_out_buf[BUFSIZE - 1] = '\n';
	send_out_buf[BUFSIZE] = '\0';
	len = BUFSIZE;
    }
    else
    {
	send_out_buf[len] = '\n';
	send_out_buf[len+1] = '\0';
	len++;
    }

    /* g_fprintf(stderr, ">: %s", send_out_buf); */

    g_mutex_lock(me.writebuf_mutex);
    me.sendQ = g_string_append(me.sendQ, send_out_buf);
    g_mutex_unlock(me.writebuf_mutex);

    g_mutex_lock(me.tag_mutex);
    if(me.send_tag == NULL)
    {
	me.send_tag = g_source_add(me.handle, G_IO_OUT | G_IO_ERR, (GIOFunc)net_send_callback);
    }
    g_mutex_unlock(me.tag_mutex);
}
