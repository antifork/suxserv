#include "sux.h"
#include "main.h"
#include "network.h"
#include "parse.h"

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
	    fatal("gethostbyname()");
	memcpy(&sock.sin_addr, he->h_addr_list[0], he->h_length);
    }

    sock.sin_port = port;
    sock.sin_family = AF_INET;

    warn("connecting to %s ... ", inet_ntop(AF_INET,
		(const void *) &sock.sin_addr, hostbuf, HOSTLEN-1));

    fflush(stderr);

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	fatal("socket()");

    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = 0;
    my_addr.sin_family = AF_INET;

    if(bind(fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) < 0)
	fatal("bind()");

    if(connect(fd, (struct sockaddr *)&sock, sizeof(struct sockaddr_in)) < 0)
	fatal("connect()");

    if(getsockname(fd, (struct sockaddr *)&my_addr, &namelen) < 0)
	fatal("getsockname()");

    if((nb = fcntl(fd, F_GETFL, 0)) < 0)
	fatal("fcntl(%d, F_GETFL, 0)", fd);
    else if(fcntl(fd, F_SETFL, nb | O_NONBLOCK) < 0)
	fatal("fcntl(%d, F_SETFL, nb | O_NONBLOCK)", fd);

    warn(" connected.\n");

    if((ret = g_io_channel_unix_new(fd)))
    {
	GError *err = NULL;
	g_io_channel_set_encoding(ret, NULL, &err);
	g_io_channel_set_buffered(ret, TRUE);
	g_io_channel_set_buffer_size(ret, IOBUFSIZE);
    }

    return(ret);
}

void send_out(gchar *fmt, ...)
{
    va_list ap;
    gchar buffer[BUFSIZE + 1];
    size_t len;

    g_return_if_fail(fmt != NULL);
    
    va_start(ap, fmt);
    len = vsnprintf(buffer, BUFSIZE, fmt, ap);
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

    if(net_transmit(me.handle, buffer, len) < 0)
	fatal("write error");

    return;
}

gint net_transmit(GIOChannel *handle, const gchar *data, gint len)
{
    guint ret;
    GError *err = NULL;
    GIOStatus status;

    g_return_val_if_fail(handle != NULL, -1);
    g_return_val_if_fail(data != NULL, -1);

    status = g_io_channel_write_chars(handle, (const gchar *) data, len, &ret, &err);

    if(status == G_IO_STATUS_ERROR)
    {
	fatal(err->message);
	g_error_free(err);
    }

    if(me.send_tag == -1)
    {
	me.send_tag = g_io_add_watch(me.handle,
		G_IO_OUT | G_IO_ERR, (GIOFunc) net_send_callback, NULL);
    }

    if (status == G_IO_STATUS_AGAIN || (errno == EINTR || errno == EPIPE))
    {
	return 0;
    }

    return status == G_IO_STATUS_NORMAL ? (gint)ret : -1;
}

gchar *net_receive(GIOChannel *handle, gsize *arnold) /* the terminator */
{
    /* recv func */

    GError *err = NULL;
    gchar *ret;
    gsize len;

    g_return_val_if_fail(handle != NULL, FALSE);

    switch(g_io_channel_read_line(handle, &ret, &len, arnold, &err))
    {
	case G_IO_STATUS_NORMAL:
	    if(ret != NULL)
	    {
		ret[*arnold] = '\0';
		return ret;
	    }
	    return NULL;

	case G_IO_STATUS_ERROR:
	    fatal(err->message);
	    g_error_free(err);
	    return NULL;
    
	case G_IO_STATUS_EOF:
	    fatal("connection closed");
	    return NULL;

	case G_IO_STATUS_AGAIN:
	    return NULL;

	default:
	    fatal("unknown status returned by net_receive() [this is a bug !]");
	    return NULL;
    }
    
    abort();
    return NULL;
}

gboolean net_flush(GIOChannel *dest)
{
    GError *err = NULL;

    switch(g_io_channel_flush(dest, &err))
    {
	case G_IO_STATUS_ERROR:
	    fatal(err->message);
	    return FALSE;
	case G_IO_STATUS_NORMAL:
	    g_source_remove(me.send_tag);
	    me.send_tag = -1;
	case G_IO_STATUS_AGAIN:
	case G_IO_STATUS_EOF:
	    return TRUE;
    }

    return FALSE;
}

gboolean net_receive_callback(GIOChannel *source)
{
    gchar *string = NULL;
    gsize arnold;

    g_return_val_if_fail(source != NULL, FALSE);

    while((string = net_receive(source, &arnold)))
    {
	string[arnold] = '\0';
	parse(string);
	g_free(string);
    }
    
    return TRUE;
}

gboolean net_send_callback(GIOChannel *dest)
{
    g_return_val_if_fail(dest != NULL, FALSE);

    return net_flush(dest);
}

gboolean net_err_callback(GIOChannel *dest)
{
    g_return_val_if_fail(dest != NULL, FALSE);

    fatal("network error");

    return FALSE;
}