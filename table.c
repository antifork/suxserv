#include "services.h"
#include "memory.h"
#include "main.h"
#define TABLE
#include "table.h"
#include "match.h"

/*
 * Fowler / Noll / Vo (FNV) Hash . .
 * Noll, Landon CUrt (LCN2)
 * chongo <was here> /\../\
 *
 * #define HASH(sta, end, hash)  while(end != sta) { hash = ((hash * 16777619UL) ^ (*end--)) }
 */
#define HASHSIZE	1021
#define FNV_prime	16777619UL
typedef unsigned long hash_t;
static __inline hash_t hash(char *s)
{
    hash_t h = 0;
    char *e = s + strlen(s) - 1;

    while(e != s)
	h = ((h * FNV_prime) ^ *e--);

    return h % HASHSIZE;
}
    

static int usertable_init(TABLE_T *d, size_t n)
{
    d->data = xmalloc(sizeof(User*) * n);
    d->seg = SG_setup(sizeof(User), n, d);
    SG_set_err_func(fatal, d->seg);

    return 1;
}

/* 
 * usertable.get(usertable, "nick", *ptr_to_allocated_user_space);
 */
static int usertable_get(TABLE_T *d, char *name, User *u)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);

    memset((void*) u, 0x0, sizeof(*u));
    if(t[h])
    {
	if(t[h]->next)
	{
	    User *p = t[h];
	    while(p && mycmp(name, p->nick) != 0)
		p = p->next;
	    if(p)
	    {
		*u = *p;
		return 1;
	    }
	    return 0;
	}
	if(mycmp(name, t[h]->nick) == 0)
	{
	    *u = *t[h];
	    return 1;
	}
    }
    return 0;
}

static User *usertable_getp(TABLE_T *d, char *name)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);
    if(t[h])
    {
	if(t[h]->next)
	{
	    User *p = t[h];
	    while(p && mycmp(name, p->nick) != 0)
		p = p->next;
	    return p;
	}
	if(mycmp(name, t[h]->nick) == 0)
	    return t[h];
    }
    return NULL;
}

static int usertable_put(TABLE_T *d, char *name, const User *u)
{
    User **t = (User **)d->data;
    hash_t  h = hash(name);

    if(t[h] == NULL)
    {
	t[h] = SG_malloc(d->seg);
	memcpy((void*)t[h], (void*)u, sizeof(*u));
	return 1;
    }
    else
    {
	User *p = t[h];
	t[h] = SG_malloc(d->seg);
	memcpy((void*)t[h], (void*)u, sizeof(*u));
	t[h]->next = p;
	return 1;
    }

    return 0;
}	

static int usertable_del(TABLE_T *d, char *name)
{
    User **t = (User**)d->data;
    hash_t h = hash(name);
    
    if(t[h])
    {
	if(t[h]->next)
	{
	    User *p = t[h], *p2 = t[h];
	    while(p && mycmp(name, p->nick) != 0)
	    {
		p2 = p;
		p = p->next;
	    }
	    if(p)
	    {
		SG_free((void *)p, d->seg);
		p2 = NULL;
		return 1;
	    }
	    return 0;
	}
	if(mycmp(name, t[h]->nick) == 0)
	{
	    SG_free((void *)t[h], d->seg);
	    t[h] = NULL;
	    return 1;
	}
    }
    return 0;
}

static int usertable_offupd(TABLE_T *d, off_t offset)
{
    int i;
    User **t = (User **)d->data;
    User *p;
    for(i = 0; i < HASHSIZE; i++)
    {
	if(!(p = t[i]))
	    continue;
	(void*)t[i] -= offset;
	for(; p->next; p = p->next)
	    (void*)p->next -= offset;
	
    }
    return 1;
}

static int usertable_clean(TABLE_T *d)
{
    free(d->data);
    SG_destroy(d->seg);
    d->data = NULL;
    d->seg = NULL;
    return 1;
}
    
static User *usertable_alloc(TABLE_T *d, char *name)
{
    User **t = (User **)d->data;
    hash_t h = hash(name);
    
    if(t[h] == NULL)
    {
	t[h] = SG_malloc(d->seg);
	return t[h];
    }
    else
    {
	User *p = t[h];
	t[h] = SG_malloc(d->seg);
	t[h]->next = p;
	return t[h];
    }
    return NULL;
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
