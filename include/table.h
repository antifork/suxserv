#ifndef __generic_table_h__
#define __generic_table_h__

#include "sux.h"

extern gint errno;

typedef struct TABLE TABLE_T;

/* AUTHOR NOTE:
 *
 * this is the way to render C++ useless and use
 * polymorphism even in C.
 * maybe in C++ we would not have the __variables
 * and we could declare them private, and we have
 * a 'this' pointer so the TABLE_T * parameters
 * become useless .. but we would have also to
 * fight with the clumsyness of g++ and the resulting
 * slowness of the generated code.
 *
 * this, obivously, imho.
 */

/* function pointers protos for later casting.
 * be aware that those ellipsis will get you in
 * trouble if you forget YOUR function prototypes.
 */

/* this should be a function that gets as arguments
 * the key and a pointer to your data,
 * get the data from the table and store into the
 * memory pointed by your pointer.
 * e.g int mytable_get(int key);
 */
typedef gpointer (*table_get_f)();

/* this should be a function that insert a pre-allocated
 * element into the table.
 * it should accept the element as an argument.
 * e.g. mytable *mytable_put(mystruct *s);
 */
typedef void (*table_put_f)();

/* this should be a function that returns a pointer
 * to the allocated data into the table.
 * it should accept as an argument the key,
 * and return the pointer.
 * e.g. mytable *mytable_alloc(int key);
 */
typedef gpointer (*table_alloc_f)();

/* this should be a function that deallocates an element
 * that previously was into the table.
 * it should accept as arguments the element to deallocate.
 * e.g. mytable *mytable_destroy(mystruct *s);
 */
typedef gpointer (*table_destroy_f)();

/* this should be a function to delete an element from
 * the table.
 * it should accept as a parameter the element to remove.
 * e.g.: mytable *mytable_del(TABLE_T *t, mystruct *s);
 */
typedef gboolean (*table_del_f)();

/* this MUST be the function to call when the table
 * data must be cleaned ..
 * parameters: none.
 */
typedef void (*table_clean_f)(void);

typedef gint (*table_count_f)(void);

struct TABLE
{	
    table_get_f get;

    table_put_f put;
    table_alloc_f alloc;

    table_del_f del;
    table_destroy_f destroy;

    table_clean_f clean;
    
    table_count_f count;

    GHashTable *__hash_tbl;
    GMemChunk *__mem_pool;
};

#define GET_FUNC(NAME, DATA_TYPE, KEY_TYPE)								\
	G_INLINE_FUNC DATA_TYPE * NAME##_table_get(KEY_TYPE *__ptr)					\
	{												\
	    return g_hash_table_lookup(NAME##_table.__hash_tbl, __ptr);					\
	}

#define ALLOC_FUNC(NAME, DATA_TYPE, KEY_NAME, KEY_TYPE)							\
	G_INLINE_FUNC gpointer NAME##_table_alloc(KEY_TYPE *__ptr)					\
	{												\
	    DATA_TYPE *__p;										\
	    if((__p = g_hash_table_lookup(NAME##_table.__hash_tbl, __ptr)))				\
		return NULL;										\
	    __p = g_mem_chunk_alloc0(NAME##_table.__mem_pool);						\
	    strcpy(__p->KEY_NAME, __ptr);								\
	    g_hash_table_insert(NAME##_table.__hash_tbl, __p->KEY_NAME, __p);				\
	    return __p;											\
	}

#define PUT_FUNC(NAME, DATA_TYPE, KEY_NAME)								\
	G_INLINE_FUNC void NAME##_table_put(DATA_TYPE *__p)						\
	{												\
	    g_hash_table_insert(NAME##_table.__hash_tbl, __p->KEY_NAME, __p);				\
	}

#define DEL_FUNC(NAME, DATA_TYPE, KEY_NAME)								\
	G_INLINE_FUNC gboolean NAME##_table_del(DATA_TYPE *__p)						\
	{												\
	    return g_hash_table_remove(NAME##_table.__hash_tbl, __p->KEY_NAME);				\
	}

#define DESTROY_FUNC(NAME, DATA_TYPE)									\
	G_INLINE_FUNC void NAME##_table_destroy(DATA_TYPE *__p)						\
	{												\
	    g_mem_chunk_free(NAME##_table.__mem_pool, __p);						\
	}

#define CLEAN_FUNC(NAME)										\
	G_INLINE_FUNC void NAME##_table_clean(void)							\
	{												\
	    g_hash_table_foreach(NAME##_table.__hash_tbl,						\
		    (GHFunc)hash_tbl_dealloc, NAME##_table.__mem_pool);					\
	    g_hash_table_destroy(NAME##_table.__hash_tbl);						\
	    g_mem_chunk_destroy(NAME##_table.__mem_pool);						\
	}

#define COUNT_FUNC(NAME)										\
	G_INLINE_FUNC gint NAME##_table_count(void)							\
	{												\
	    return g_hash_table_size(NAME##_table.__hash_tbl);						\
	}

#define SETUP_FUNC(NAME, DATA_TYPE, HASH_FUNC)								\
	void NAME##_table_setup(gint PRE_ALLOC)								\
	{												\
	    NAME##_table.__mem_pool = g_mem_chunk_create(DATA_TYPE, PRE_ALLOC, G_ALLOC_AND_FREE);	\
	    NAME##_table.__hash_tbl = g_hash_table_new((GHashFunc)HASH_FUNC, __sux_hash_strcmp);	\
	    												\
	    NAME##_table.get = (table_get_f)NAME##_table_get;						\
	    NAME##_table.put = (table_put_f)NAME##_table_put;						\
	    NAME##_table.alloc = (table_alloc_f)NAME##_table_alloc;					\
	    NAME##_table.del = (table_del_f)NAME##_table_del;						\
	    NAME##_table.destroy = (table_destroy_f)NAME##_table_destroy;				\
	    NAME##_table.clean = (table_clean_f)NAME##_table_clean;					\
	    NAME##_table.count = (table_count_f)NAME##_table_count;				\
	}

#define LOCAL_TABLE_INSTANCE(NAME)	TABLE_T NAME##_table;
#define REMOTE_TABLE_INSTANCE(NAME)	extern TABLE_T NAME##_table;

#define TABLE_DECLARE(NAME, DATA_TYPE, HASH_FUNC, KEY_NAME, KEY_TYPE);	\
	LOCAL_TABLE_INSTANCE(NAME)					\
	GET_FUNC(NAME, DATA_TYPE, KEY_TYPE)				\
	ALLOC_FUNC(NAME, DATA_TYPE, KEY_NAME, KEY_TYPE)			\
	PUT_FUNC(NAME, DATA_TYPE, KEY_NAME)				\
	DEL_FUNC(NAME, DATA_TYPE, KEY_NAME)				\
	DESTROY_FUNC(NAME, DATA_TYPE)					\
	CLEAN_FUNC(NAME)						\
	COUNT_FUNC(NAME)						\
	SETUP_FUNC(NAME, DATA_TYPE, HASH_FUNC)

#define TABLE_SETUP_FUNC(NAME, PRE_ALLOC)	NAME##_table_setup(PRE_ALLOC)

#define _TBL(NAME)		(NAME##_table)
#define _MPL(NAME)		(NAME##_pool)

#define LOCAL_MEMPOOL_INSTANCE(NAME)		GMemChunk *NAME##_pool
#define REMOTE_MEMPOOL_INSTANCE(NAME)		extern GMemChunk *NAME##_pool
#define MEMPOOL_DECLARE				LOCAL_MEMPOOL_INSTANCE

#define MEMPOOL_SETUP_FUNC(NAME, DATA_TYPE, PRE_ALLOC)	\
		NAME##_pool = g_mem_chunk_create(DATA_TYPE, PRE_ALLOC, G_ALLOC_AND_FREE)

void setup_tables(void);

#endif
