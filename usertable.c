#include "sux.h"
#include "memory.h"
#include "main.h"
#define USERTABLE
#include "usertable.h"
#include "match.h"

/*
 * Fowler / Noll / Vo (FNV) Hash . .
 * Noll, Landon CUrt (LCN2)
 * chongo <was here> /\../\
 *
 * #define HASH(sta, end, hash)  while(end != sta) { hash = ((hash * 16777619UL) ^ (*end--)) }
 */
#define HASHSIZE	1021
#define MEM_CHUNK	256
#define FNV_prime	16777619UL
typedef unsigned long hash_t;
#ifdef G_CAN_INLINE
G_INLINE_FUNC
#endif
static hash_t hash(gchar *s)
{
    hash_t h = 0;
    gchar *e = s + strlen(s) - 1;

    while(e != s)
	h = ((h * FNV_prime) ^ *e--);

    return h % HASHSIZE;
}
    

static gint usertable_init(TABLE_T *d, size_t n)
{
    d->data = g_new0(User*, n);
    d->seg = SG_setup(sizeof(User), MEM_CHUNK, d);
    SG_set_err_func(fatal, d->seg);

    return 1;
}

/* 
 * usertable.get(usertable, "nick", *ptr_to_allocated_user_space);
 */
static gint usertable_get(TABLE_T *d, gchar *name, User *u)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);
    User *p = t[h];

    memset((void*) u, 0x0, sizeof(User));
    if(p)
    {
	while(p && mycmp(name, p->nick) != 0)
	{
	    p = p->next;
	}
	if(p)
	{
	    *u = *p;
	    return 1;
	}
    }

    return 0;
}

static User *usertable_getp(TABLE_T *d, gchar *name)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);
    User *p = t[h];
    
    if(p)
    {
	while(p && mycmp(name, p->nick) != 0)
	    p = p->next;
    }
    return p;
}

static gint usertable_put(TABLE_T *d, gchar *name, User *u)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);
    User *p = t[h];

    t[h] = SG_malloc(d->seg);
    *(t[h]) = *u;
    t[h]->next = p;

    return 1;
}	

static gint usertable_del(TABLE_T *d, gchar *name)
{
    User **t = (User**)d->data;
    hash_t h = hash(name);
    User *p = t[h], *p2 = NULL;

    while(p && mycmp(name, p->nick) != 0)
    {
	p2 = p;
	p = p->next;
    }
    if(p)
    {
	/* ok, found. */
	if(p == t[h]) /* was the first entry .. */
	    t[h] = p->next;
	else if(p2 != NULL) /* was any entry in the middle, or the last entry .. */
	    p2->next = p->next;

	SG_free(p, d->seg);
	return 1;
    }
    return 0;
}

static gint usertable_offupd(TABLE_T *d, off_t offset)
{
    gint i;
    User **t = (User **)d->data;
    User *p;

#ifdef DEBUG
    void *start, *end;
    start = d->seg->__data;
    end = d->seg->__data + (d->seg->pagesize * d->seg->numpages);
#endif

    for(i = 0; i < HASHSIZE; i++)
    {
	if(t[i] == NULL)
	    continue;

	((void*)t[i]) += offset;
	p = t[i];
	while(p->next)
	{
	    ((void*)p->next) += offset;
	    
#ifdef DEBUG
	    if((void*)p->next < start || (void*)p->next > end)
	    {
		fprintf(stderr, "usertable_offupd created an OOB pointer ...");
		raise(SIGSEGV);
	    }
#endif
	    p = p->next;
	}
    }
#if 0
    printf("data [%p <-> %p]\n", start, end);
    for(i = 0; i < HASHSIZE; i++)
    {
	printf("t[%d] =>", i);
	for(p = t[i]; p; p = p->next)
	{
	    printf(" %p", p);
	    if((void*)p < start || (void*)p > end)
		printf(" \033[1;31mOOB !!!!\033[0m");
	}
	printf("\n");
    }
#endif
    return 1;
}

static gint usertable_clean(TABLE_T *d)
{
    free(d->data);
    SG_destroy(d->seg);
    d->data = NULL;
    d->seg = NULL;
    return 1;
}
    
static User *usertable_alloc(TABLE_T *d, gchar *name)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);
    User *p = t[h];

    t[h] = SG_malloc(d->seg);
    t[h]->next = p;

    return t[h];
}

void tables_init(void)
{
    usertable.get = (table_get_f)usertable_get;
    usertable.getp = (table_getp_f)usertable_getp;
    usertable.put = (table_put_f)usertable_put;
    usertable.del = (table_del_f)usertable_del;
    usertable.alloc = (table_alloc_f)usertable_alloc;
    usertable.init = usertable_init;
    usertable.offupd = usertable_offupd;
    usertable.clean = usertable_clean;

    usertable.init(&usertable, HASHSIZE);
}
