#ifndef __network_h__
#define __network_h__

GIOChannel *connect_server(gchar *, guint);

gboolean net_send_callback(GIOChannel *);
gboolean net_receive_callback(GIOChannel *);
gboolean net_err_callback(GIOChannel *);

gint net_transmit(GIOChannel *, const gchar *, gint);
gchar *net_receive(GIOChannel *, gsize *);
gboolean net_flush(GIOChannel *);
gboolean net_shutdown(GIOChannel *);

void send_out(gchar *, ...) G_GNUC_PRINTF(1, 2);

#endif
