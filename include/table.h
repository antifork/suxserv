#include "sux.h"

#ifndef __generic_table_h__
#define __generic_table_h__

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

/* this MUST be the function to initialize the table, in
 * which you have to allocate the segment and the void
 * data pointer.
 * parameters: none
 */
typedef void (*table_init_f)(void);

struct TABLE
{	
    table_get_f get;

    table_put_f put;
    table_alloc_f alloc;

    table_del_f del;
    table_destroy_f destroy;

    table_clean_f clean;

    table_init_f init;

    GHashTable *__hash_tbl;
    GMemChunk *__mem_pool;
};

#define MEM_CHUNK	512

#define INIT_FUNC(NAME, DATA_TYPE, HASH_FUNC)								\
	G_INLINE_FUNC void NAME##table_init(void)							\
	{												\
	    NAME##table.__mem_pool = g_mem_chunk_create(DATA_TYPE, MEM_CHUNK, G_ALLOC_AND_FREE);	\
	    NAME##table.__hash_tbl = g_hash_table_new((GHashFunc)HASH_FUNC, g_str_equal);		\
	}

#define GET_FUNC(NAME, DATA_TYPE, KEY_TYPE)								\
	G_INLINE_FUNC DATA_TYPE * NAME##table_get(KEY_TYPE *__ptr)					\
	{												\
	    return g_hash_table_lookup(NAME##table.__hash_tbl, __ptr);					\
	}

#define ALLOC_FUNC(NAME, DATA_TYPE, KEY_NAME, KEY_TYPE)							\
	G_INLINE_FUNC gpointer NAME##table_alloc(KEY_TYPE *__ptr)					\
	{												\
	    DATA_TYPE *__p;										\
	    if((__p = g_hash_table_lookup(NAME##table.__hash_tbl, __ptr)))				\
		return NULL;										\
	    __p = g_mem_chunk_alloc0(NAME##table.__mem_pool);						\
	    strcpy(__p->KEY_NAME, __ptr);								\
	    g_hash_table_insert(NAME##table.__hash_tbl, __p->KEY_NAME, __p);			\
	    return __p;											\
	}

#define PUT_FUNC(NAME, DATA_TYPE, KEY_NAME)								\
	G_INLINE_FUNC void NAME##table_put(DATA_TYPE *__p)						\
	{												\
	    g_hash_table_insert(NAME##table.__hash_tbl, __p->KEY_NAME, __p);			\
	}

#define DEL_FUNC(NAME, DATA_TYPE, KEY_NAME)								\
	G_INLINE_FUNC gboolean NAME##table_del(DATA_TYPE *__p)						\
	{												\
	    return g_hash_table_remove(NAME##table.__hash_tbl, __p->KEY_NAME);			\
	}

#define DESTROY_FUNC(NAME, DATA_TYPE)									\
	G_INLINE_FUNC void NAME##table_destroy(DATA_TYPE *__p)						\
	{												\
	    g_mem_chunk_free(NAME##table.__mem_pool, __p);						\
	}

#define CLEAN_FUNC(NAME)										\
	G_INLINE_FUNC void NAME##table_clean(void)							\
	{												\
	    g_hash_table_destroy(NAME##table.__hash_tbl);						\
	    g_mem_chunk_destroy(NAME##table.__mem_pool);						\
	}

#define SETUP_FUNC(NAME)										\
	void NAME##table_setup(void)									\
	{												\
	    NAME##table.get = (table_get_f)NAME##table_get;						\
	    NAME##table.put = (table_put_f)NAME##table_put;						\
	    NAME##table.alloc = (table_alloc_f)NAME##table_alloc;					\
	    NAME##table.del = (table_del_f)NAME##table_del;						\
	    NAME##table.destroy = (table_destroy_f)NAME##table_destroy;					\
	    NAME##table.init = (table_init_f)NAME##table_init;						\
	    NAME##table.clean = (table_clean_f)NAME##table_clean;					\
	    												\
	    NAME##table.init();										\
	}

#define LOCAL_TABLE_INSTANCE(NAME)	TABLE_T NAME##table;
#define REMOTE_TABLE_INSTANCE(NAME)	extern TABLE_T NAME##table;

#define TABLE_DECLARE(NAME, DATA_TYPE, HASH_FUNC, KEY_NAME, KEY_TYPE)	\
	LOCAL_TABLE_INSTANCE(NAME)					\
	INIT_FUNC(NAME, DATA_TYPE, HASH_FUNC)				\
	GET_FUNC(NAME, DATA_TYPE, KEY_TYPE)				\
	ALLOC_FUNC(NAME, DATA_TYPE, KEY_NAME, KEY_TYPE)			\
	PUT_FUNC(NAME, DATA_TYPE, KEY_NAME)				\
	DEL_FUNC(NAME, DATA_TYPE, KEY_NAME)				\
	DESTROY_FUNC(NAME, DATA_TYPE)					\
	CLEAN_FUNC(NAME)						\
	SETUP_FUNC(NAME)						\

#define TABLE_SETUP_FUNC(NAME)	NAME##table_setup()
    

void tables_init(void);

#endif
