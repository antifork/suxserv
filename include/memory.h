#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#ifndef __memory_h__
#define __memory_h__

extern int errno;

/* 
 * 19:31 <awgn> r -> indirizzo del segmento di bitmap
 * 19:32 <awgn> b -> bittesimo bit
 * 19:32 <awgn> REG_BS == setta
 * 19:32 <vjt> fico
 * 19:32 <awgn> REG_BR resetta
 * 19:32 <awgn> REG_BT == testa
 * 19:32 <awgn> REG_BN == inverte
 * 19:34 <awgn> se 
 * 19:34 <awgn> usi
 * 19:34 <awgn> un array di long
 * 19:34 <awgn> allora
 * 19:34 <awgn> devi cabiare
 * 19:34 <awgn> >>3 ( /8 ) con /32
 * 19:34 <awgn> che sarebnbe
 * 19:35 <awgn> >>5
 * 19:35 <awgn> e
 * 19:35 <awgn> &7 ( che sarebbe MOD 8 ) con MOD 32 che e' &31
 */
typedef int bitmap_t;
#define BMP_BS(r,b)      ( r[b>>5] |=   1<<(b&31) )
#define BMP_BR(r,b)      ( r[b>>5] &= ~ 1<<(b&31) )
#define BMP_BT(r,b)      ( r[b>>5]  &   1<<(b&31) )
#define BMP_BN(r,b)      ( r[b>>5] ^=   1<<(b&31) )
#define BITSIZE(x)       ( x << 3 )

typedef struct TABLE TABLE_T;
typedef struct SEG SEG_T;

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
 * the table, the key and a pointer to your data,
 * get the data from the table and store into the
 * memory pointed by your pointer.
 * e.g int mytable_get(TABLE_T *t, int key, mytable *ret);
 */
typedef int (*table_get_f)(TABLE_T*, ...);
/* this should be a function that returns a pointer
 * to the allocated data into the table.
 * it should accept as arguments the table and a key,
 * and return the pointer.
 * e.g. mytable *mytable_getp(TABLE_T *t, int key);
 */
typedef void *(*table_getp_f)(TABLE_T*, ...);
/* this should be a function to put data into the table.
 * it should accept as arguments the table, the key
 * and a const pointer to the stored data.
 * e.g. int mytable_put(TABLE_T *, int key, const mytable *elem);
 */
typedef int (*table_put_f)(TABLE_T *, ...);
/* this should be a function to allocate an element into
 * the table and return the pointer to it.
 * it should accept as parameters the table and the key,
 * and return the pointer.
 * e.g.: mytable *mytable_alloc(TABLE_T *t, int key);
 */
typedef void *(*table_alloc_f)(TABLE_T *, ...);
/* this should be a function to delete some data from the
 * table.
 * it should accept as arguments the table and the key.
 * e.g. int mytable_del(TABLE_T *, int key);
 */
typedef int (*table_del_f)(TABLE_T *, ...);
/* this MUST be the function to update the function pointers
 * when we realloc the segment.
 * parameters: the table and the offset.
 * you do not need to write a function different than
 * the one prototyped here, unless you have a really
 * complex datatype.
 */
typedef int (*table_offupd_f)(TABLE_T *, off_t);
/* this MUST be the function to initialize the table, in
 * which you have to allocate the segment and the void
 * data pointer.
 * parameters: the table and the number of elements.
 */
typedef int (*table_init_f)(TABLE_T *, size_t);
/* this MUST be the function to call when the table
 * data must be cleaned ..
 * parameter: the table.
 */
typedef int (*table_clean_f)(TABLE_T *);

struct TABLE
{	
	table_get_f get;
	table_getp_f getp;
	table_put_f put;
	table_del_f del;
	table_offupd_f offupd;
	table_init_f init;
	table_clean_f clean;
	table_alloc_f alloc;
	void *data; /* ptr to the data structure */
	SEG_T *seg; /* ptr to the allocator structure */
};

struct SEG
{
	size_t numpages;
	size_t pagesize;
	size_t datasize;
	size_t bitmapsize;
	long __firstfree;
	bitmap_t *__bitmap;
	TABLE_T *__table;
	void *__data;
	void (*errfunc)(char *, ...);
};

SEG_T *SG_setup(size_t, size_t, TABLE_T *);
void *SG_malloc(SEG_T *seg);
void SG_free(void *p, SEG_T *seg);
void SG_set_err_func(void (*)(char *, ...), SEG_T *seg);
void SG_destroy(SEG_T *seg);

#endif
