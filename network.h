#ifndef __network_h__
#define __network_h__

GIOChannel *connect_server(gchar *, unsigned gint);
gboolean net_send_callback(GIOChannel *, GIOCondition);
gboolean net_receive_callback(GIOChannel *, GIOCondition);
gboolean net_err_callback(GIOChannel *, GIOCondition);
int net_transmit(GIOChannel *, const gchar *, gint);
void send_out(gchar *, ...);

#endif
