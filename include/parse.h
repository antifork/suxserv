#ifndef __parse_h__
#define __parse_h__

#define MAXPARA 15

#include "sux.h"

gint parse(gchar *);

void send_message_count(gchar *);

gchar **my_g_strsplit(gchar *, gchar, gint *);

#endif
