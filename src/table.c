#include "sux.h"
#include "table.h"
#include "main.h"
#include "match.h"

#define MEM_CHUNK	512

/*
 * Fowler / Noll / Vo (FNV) Hash . .
 * Noll, Landon CUrt (LCN2)
 * chongo <was here> /\../\
 *
 * #define HASH(sta, end, hash)  while(end != sta) { hash = ((hash * 16777619UL) ^ (*end--)) }
 */
#define FNV_PRIME	16777619UL

G_INLINE_FUNC guint FNV_hash(guchar *s)
{
    guint h = 0;
    guchar *e = s + strlen(s) - 1;

    while(e != s)
	h = ((h * FNV_PRIME) ^ *e--);

    return h;
}

TABLE_DECLARE(user, User, FNV_hash, nick, gchar)
TABLE_DECLARE(channel, Channel, FNV_hash, chname, gchar)
    
void tables_init(void)
{
    TABLE_SETUP_FUNC(user);
    TABLE_SETUP_FUNC(channel);
}