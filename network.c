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

    if(!inet_pton(AF_INET, host, (void*)&sock.sin_addr))
    {
	struct hostent *he = gethostbyname(host);
	if(!he)
	    fatal("gethostbyname()");
	memcpy(&sock.sin_addr, he->h_addr_list[0], he->h_length);
    }

    sock.sin_port = port;
    sock.sin_family = AF_INET;

    printf("connecting to %s ...", inet_ntop(AF_INET,
		(const void *) &sock.sin_addr, hostbuf, HOSTLEN-1));

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

    printf(" connected.\n");

    if((ret = g_io_channel_unix_new(fd)))
    {
	GError *err = NULL;
	g_io_channel_set_encoding(ret, NULL, &err);
	g_io_channel_set_buffered(ret, TRUE);
	g_io_channel_set_buffer_size(ret, 32768);
    }

    return(ret);
}

/* Transmit data, return number of bytes sent, -1 = error */
/* stolen from irssi */
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

void send_out(gchar *fmt, ...)
{
    va_list ap;
    gchar buffer[BUFSIZE + 1];
    size_t len;
    
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

gboolean net_receive_callback(GIOChannel *source, GIOCondition cond)
{
    /* recv callback func */

    GError *err = NULL;
    gchar *string;
    gsize len, arnold; /* terminator */
    GIOStatus status;

    while((status = g_io_channel_read_line(source, &string, &len, &arnold, &err)) == G_IO_STATUS_NORMAL)
    {
	if(string != NULL)
	{
	    string[arnold] = '\0';
	    parse(string);
	    g_free(string);
	}
    }
    
    if(status == G_IO_STATUS_ERROR)
    {
	fprintf(stderr, err->message);
	g_error_free(err);
	return FALSE;
    }
    else if(status == G_IO_STATUS_EOF)
    {
	fprintf(stderr, "connection closed");
	fatal("sux");
	return FALSE;
    }
    
    return TRUE;
}

gboolean net_send_callback(GIOChannel *dest, GIOCondition cond)
{
    GError *err = NULL;
    
    switch(g_io_channel_flush(dest, &err))
    {
	case G_IO_STATUS_ERROR:
	    fatal(err->message);
	case G_IO_STATUS_NORMAL:
	    g_source_remove(me.send_tag);
	    me.send_tag = -1;
	case G_IO_STATUS_AGAIN:
	case G_IO_STATUS_EOF:
	    return TRUE;
    }

    return TRUE;
}

gboolean net_err_callback(GIOChannel *dest, GIOCondition cond)
{
    /* err func */
    fatal("network error");

    return FALSE;
}
