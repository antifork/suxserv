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

#endif
