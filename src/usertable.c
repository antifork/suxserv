#include "sux.h"
#include "table.h"
#include "main.h"
#define USERTABLE
#include "usertable.h"
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

G_INLINE_FUNC void usertable_init(void)
{
    usertable.__mem_pool = g_mem_chunk_create(User, MEM_CHUNK, G_ALLOC_AND_FREE);
    usertable.__hash_tbl = g_hash_table_new((GHashFunc)FNV_hash, g_str_equal);

    if(!usertable.__mem_pool || !usertable.__hash_tbl)
	g_error("unable to setup the usertable");
}

G_INLINE_FUNC User *usertable_get(gchar *name)
{
    return g_hash_table_lookup(usertable.__hash_tbl, name);
}

G_INLINE_FUNC gpointer usertable_alloc(gchar *name)
{
    User *p;

    if((p = g_hash_table_lookup(usertable.__hash_tbl, name)))
    {
	return NULL;
    }

    p = g_mem_chunk_alloc0(usertable.__mem_pool);
    strcpy(p->nick, name);

    g_hash_table_insert(usertable.__hash_tbl, p->nick, p);

    return p;
}

G_INLINE_FUNC void usertable_put(User *u)
{
    g_hash_table_insert(usertable.__hash_tbl, u->nick, u);
}

G_INLINE_FUNC gboolean usertable_del(User *u)
{
    return g_hash_table_remove(usertable.__hash_tbl, u->nick);
}

G_INLINE_FUNC void usertable_destroy(User *u)
{
    g_mem_chunk_free(usertable.__mem_pool, u);
}

G_INLINE_FUNC void usertable_clean(void)
{
    g_hash_table_destroy(usertable.__hash_tbl);
    g_mem_chunk_destroy(usertable.__mem_pool);

    usertable.__hash_tbl = NULL;
    usertable.__mem_pool = NULL;
}
    
void tables_init(void)
{
    usertable.get = (table_get_f)usertable_get;
    usertable.put = (table_put_f)usertable_put;
    usertable.alloc = (table_alloc_f)usertable_alloc;
    usertable.del = (table_del_f)usertable_del;
    usertable.destroy = (table_destroy_f)usertable_destroy;
    usertable.init = usertable_init;
    usertable.clean = usertable_clean;

    usertable.init();
}
