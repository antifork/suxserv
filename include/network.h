#ifndef __network_h__
#define __network_h__

#define IOBUFSIZE 32768

GIOChannel *connect_server(gchar *, guint);

gboolean net_send_callback(GIOChannel *);
gboolean net_receive_callback(GIOChannel *);
gboolean net_err_callback(GIOChannel *);

gboolean net_shutdown(GIOChannel *);

void setup_netbuf(void);

void send_out(gchar *, ...) G_GNUC_PRINTF(1, 2);

GSource *g_input_add(GIOChannel *, GIOCondition, GIOFunc);
GSource *g_timeout_source_add(guint, GSourceFunc, gpointer);
gboolean g_source_del(GSource *);

#endif
