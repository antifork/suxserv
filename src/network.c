#include "sux.h"
#include "main.h"
#include "network.h"
#include "parse.h"
#include "log.h"

GIOChannel *connect_server(gchar *host, guint port)
{
    struct sockaddr_in sock, my_addr;
    gint fd, nb;
    socklen_t namelen = sizeof(my_addr);
    gchar hostbuf[HOSTLEN + 1];
    GIOChannel *ret;

    g_return_val_if_fail(host != NULL, NULL);
    g_return_val_if_fail(port > 0, NULL);

    if(!inet_pton(AF_INET, host, (void*)&sock.sin_addr))
    {
	struct hostent *he = gethostbyname(host);
	if(!he)
	    g_errno_critical("gethostbyname()");
	memcpy(&sock.sin_addr, he->h_addr_list[0], he->h_length);
    }

    sock.sin_port = port;
    sock.sin_family = AF_INET;

    g_message("Connecting to %s ... ", inet_ntop(AF_INET,
		(const void *) &sock.sin_addr, hostbuf, HOSTLEN-1));

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

    if((nb = fcntl(fd, F_GETFL, 0)) < 0)
	g_errno_critical("fcntl(F_GETFL)");
    else if(fcntl(fd, F_SETFL, nb | O_NONBLOCK) < 0)
	g_errno_critical("fcntl(F_SETFL, nb | O_NONBLOCK)");

    g_message("Connection established");

    if((ret = g_io_channel_unix_new(fd)))
    {
	GError *err = NULL;
	
	g_io_channel_set_encoding(ret, NULL, &err);

	if(err)
	{
	    g_critical(err->message);
	    g_error_free(err);

	    g_io_channel_shutdown(ret, FALSE, NULL);
	    g_io_channel_unref(ret);

	    return NULL;
	}

	g_io_channel_set_flags(ret, G_IO_FLAG_NONBLOCK, &err);

	if(err)
	{
	    g_critical(err->message);
	    g_error_free(err);

	    g_io_channel_shutdown(ret, FALSE, NULL);
	    g_io_channel_unref(ret);

	    return NULL;
	}

	g_io_channel_set_buffered(ret, TRUE);
	g_io_channel_set_buffer_size(ret, IOBUFSIZE);

    }

    return(ret);
}

void send_out(gchar *fmt, ...)
{
    va_list ap;
    gchar buffer[BUFSIZE + 1];
    gsize len;

    guint ret;
    GError *err = NULL;
    GIOStatus status;

    g_return_if_fail(fmt != NULL);
    
    va_start(ap, fmt);
    len = g_vsnprintf(buffer, BUFSIZE, fmt, ap);
    va_end(ap);

    if(len > BUFSIZE - 2)
    {
	buffer[BUFSIZE - 1] = '\n';
	buffer[BUFSIZE] = '\0';
	len = BUFSIZE;
    }
    else
    {
	buffer[len] = '\n';
	buffer[len+1] = '\0';
	len++;
    }

    /* g_fprintf(stderr, ">: %s", buffer); */

    g_return_if_fail(me.handle != NULL);
    g_return_if_fail(buffer != NULL);

    status = g_io_channel_write_chars(me.handle, (const gchar *) buffer, len, &ret, &err);

    if(status == G_IO_STATUS_ERROR)
    {
	g_critical_syslog("Write error: %s", err->message);
	g_error_free(err);
    }

    if(me.send_tag == -1)
    {
	me.send_tag = g_io_add_watch(me.handle,
		G_IO_OUT | G_IO_ERR, (GIOFunc) net_send_callback, NULL);
    }
}

static GString *readbuf = NULL;

void setup_netbuf(void)
{
    readbuf = g_string_sized_new(BUFSIZE);
}

gboolean net_receive_callback(GIOChannel *handle)
{
    GError *err = NULL;
    gint arnold; /* terminator */

    g_return_val_if_fail(handle != NULL, FALSE);

    while(TRUE)
    {
	switch(g_io_channel_read_line_string(handle, readbuf, &arnold, &err))
	{
	    case G_IO_STATUS_NORMAL:
		readbuf->str[arnold] = '\0';
		parse(readbuf->str);
		continue;

	    case G_IO_STATUS_ERROR:
		g_critical_syslog("Read error: %s", err->message);
		g_error_free(err);
		return FALSE;
    
	    case G_IO_STATUS_EOF:
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

gboolean net_send_callback(GIOChannel *dest)
{
    GError *err = NULL;

    g_return_val_if_fail(dest != NULL, FALSE);

    switch(g_io_channel_flush(dest, &err))
    {
	case G_IO_STATUS_ERROR:
	    g_critical_syslog("Write error: %s", err->message);
	    g_error_free(err);
	    return FALSE;

	case G_IO_STATUS_NORMAL:
	    g_source_remove(me.send_tag);
	    me.send_tag = -1;
	    /* fallthrough */
	case G_IO_STATUS_AGAIN:
	case G_IO_STATUS_EOF:
	    return TRUE;
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
