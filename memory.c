/* insert a (C) note here.
 */

#include "sux.h"
#include "memory.h"

/*******************************************************\
 * SEGMENT FUNCTIONS
\*******************************************************/

/* leet awgn`s algorythm to find the first zero bit in
 * an integer
 */
static gint fz[] =
    { 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8
};

#ifdef G_CAN_INLINE
G_INLINE_FUNC
#endif
static gint
get_fz (gint i)
{
    register gint ret = 0;
    register gint cnt;

    /* optimize .. if we pass a -1, return it instead of cycling.. also because
     * the algorithm returns 0 when the first bit is 0 and when the bits are all
     * ones :) */
    if(i == 0xffffffff)
	return -1;

    do {
	cnt = fz[i & 0xff];
	ret += cnt;
	i = (i >> 8) & 0xffffff;
    }
    while (cnt == 8);

    return (ret);
}


/* 
 * predefined error function
 */
static void SG_err_func_predef(gchar *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);

	/*NOTREACH*/
	return;
}

/*
 * setup function for the memory segment
 */
SEG_T *SG_setup(size_t pagesize, size_t numpages, TABLE_T *table)
{
	SEG_T *ret = g_new0(SEG_T, 1);
	
	ret->pagesize = pagesize;
	ret->numpages = numpages;
	ret->datasize = pagesize * numpages;
	ret->bitmapsize = numpages >> 3; /* bitmap byte size */
	ret->__firstfree = 0L;
	ret->__bitmap = (bitmap_t*)g_malloc0(ret->bitmapsize);
	ret->__table = table;
	ret->__data = g_malloc0(ret->datasize);
	ret->errfunc = SG_err_func_predef;

	return ret;
}

/*
 * realloc function for a segment ..
 */
static void realloc_segment (SEG_T *seg)
{
	off_t offset;
	SEG_T newseg;

	newseg.pagesize = seg->pagesize;
	newseg.numpages = seg->numpages << 1;  /* XXX: do this better ..
						* as the segment grows,
						* we alloc less space.
						*/
	newseg.datasize = newseg.numpages * newseg.pagesize;
	newseg.bitmapsize = newseg.numpages >> 3;
	newseg.__firstfree = seg->numpages;
#ifdef DEBUG
	printf("realloc(%p->%p, %d) -> ", seg->__data, seg->__data + seg->datasize, newseg.datasize);
#endif	
	newseg.__data = g_realloc(seg->__data, newseg.datasize);
	memset(newseg.__data + seg->datasize, 0x0, newseg.datasize - seg->datasize);
	offset = (off_t)(newseg.__data - seg->__data);

#ifdef DEBUG
	printf("%p -> %p\n", newseg.__data, newseg.__data + newseg.datasize);
	printf("realloc(%p, %d) -> ", seg->__bitmap, newseg.bitmapsize);
#endif
	newseg.__bitmap = g_realloc((void*)seg->__bitmap, newseg.bitmapsize);
	memset(newseg.__bitmap + seg->bitmapsize, 0x0, newseg.bitmapsize - seg->bitmapsize);

#ifdef DEBUG
	printf("%p\n", newseg.__bitmap);
	printf("realloc data %d to %d, off_t: %ld\n", seg->datasize, newseg.datasize, offset);
	printf("realloc bitmap %d to %d, off_t: %d\n", seg->bitmapsize, newseg.bitmapsize, 
			((void*)seg->__bitmap) - ((void*)newseg.__bitmap));
#endif
	seg->numpages = newseg.numpages;
	seg->datasize = newseg.datasize;
	seg->bitmapsize = newseg.bitmapsize;
	seg->__firstfree = newseg.__firstfree;
	seg->__bitmap = newseg.__bitmap;
	seg->__data = newseg.__data;
	
	if(offset != 0 && seg->__table)
		seg->__table->offupd(seg->__table, offset);
	return;
}

/*
 * and, finally, our own malloc()
 */
void *SG_malloc(SEG_T *seg)
{
	void *ret;
	bitmap_t *pagefinder,
	         *bitmapend = (void*)seg->__bitmap + seg->bitmapsize;
	gint fz;
#ifdef DEBUG
	void *start, *end;
#endif
	
	if(seg->__firstfree == -1)
	{
		/* we previously ran out of space .. do the realloc() stuff. */
		realloc_segment(seg);
	}

	/* get the pointer into our allocated buffer */
	ret = seg->__data + (seg->pagesize * seg->__firstfree);

	/* get the map address of the chunk of pages */
	pagefinder = (seg->__bitmap + (seg->__firstfree / BITSIZE((sizeof(bitmap_t)))));

	/* set the bit of the page we are returning. */
	BMP_BS(pagefinder, get_fz(*pagefinder));

	/* optimize the next free page find .. we do another get_fz() on the same
	 * integer, so if the pages chunk has another page free we get it and
	 * return. */
	if((fz = get_fz(*pagefinder)) == -1)
	{
		/* we have a 0xffffffff ... */
		do
			pagefinder++;
		while(*pagefinder == 0xffffffff && pagefinder < bitmapend);
		/* if we run out of space, don`t worry. the next SG_malloc() will
		 * do the realloc_segment() stuff. */
		seg->__firstfree = (pagefinder == bitmapend) ? -1 :
			get_fz(*pagefinder) + (BITSIZE(((void*)pagefinder - (void*)seg->__bitmap)));
	}
	else
		seg->__firstfree = fz + (BITSIZE(((void*)pagefinder - (void*)seg->__bitmap)));
	
#ifdef DEBUG
	start = seg->__data;
	end = seg->__data + seg->pagesize * seg->numpages;

	if(ret < start || ret > end)
	{
	    fprintf(stderr, "SG_malloc found an OOB pointer ... and tried to return it.");
	    raise(SIGSEGV);
	}
#endif
	return ret;
}

/*
 * and, what is a malloc() without a free() ? :)
 */
void SG_free(void *p, SEG_T *seg)
{
	gint pagenum;
	bitmap_t *b;
	/* first, consistancy checks. */

	/* NULL pointer */
	if(p == NULL)
	{
		seg->errfunc("NULL pointer passed to SG_free() !!!");
		return;
	}
	/* out of bounds pointer */
	if(p < seg->__data || p > (seg->__data + seg->datasize))
	{
		seg->errfunc("OOB pointer passed to SG_free(): data -> %p -- %p, ptr -> %p",
				seg->__data, seg->__data + seg->datasize, p);
		return;
	}
	/* inconsistent pointer (into the middle of a page) */
	if((p - seg->__data) % seg->pagesize)
	{
		seg->errfunc("INCONSISTENT pointer passed to SG_free(), %d bytes off the nearest page bound",
				(p - seg->__data) % seg->pagesize); /* let`s consume less memory */
		return;
	}
	/* ok, good pointer. */

	/* get page size associated with this ptr */
	pagenum = (p - seg->__data) / seg->pagesize;
	if(seg->__firstfree == -1 || pagenum < seg->__firstfree)
		seg->__firstfree = pagenum;
	
	/* update the bitmap */
	b = (seg->__bitmap + (pagenum / BITSIZE(sizeof(bitmap_t))));
	BMP_BN(b, (pagenum % BITSIZE(sizeof(bitmap_t))));
	/*     |   \__________________________________/
	 *     /                    |
	 * chunk of pages   our page offset into the
	 * into the bitmap  chunk of pages, the one to
	 *                  set to zero
	 */

	/* zero the free()`d page */
	memset(p, 0x0, seg->pagesize);
	
	return;
}
	
/*
 * lame helper function to set the errfunc
 * pointer into the SEG struct..
 */
/* XXX: maybe put this function into SG_setup ?? */
#ifdef G_CAN_INLINE
G_INLINE_FUNC
#endif
void SG_set_err_func(void (*errfunc)(gchar *, ...), SEG_T *seg)
{
	seg->errfunc = errfunc;
	return;
}

/*
 * a lame way to call free() :)
 */
#ifdef G_CAN_INLINE
G_INLINE_FUNC
#endif
void SG_destroy(SEG_T *seg)
{
	g_free((void *)seg);
	return;
}
