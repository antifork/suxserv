#ifndef __network_h__
#define __network_h__

GIOChannel *connect_server(gchar *, guint);

gboolean net_send_callback(GIOChannel *);
gboolean net_receive_callback(GIOChannel *);
gboolean net_err_callback(GIOChannel *);

gboolean net_shutdown(GIOChannel *);

void setup_netbuf(void);

void send_out(gchar *, ...) G_GNUC_PRINTF(1, 2);

#endif
