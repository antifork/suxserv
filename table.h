#ifndef __table_h__
#define __table_h__

#ifndef TABLE
# define EXTERN extern
#else
# define EXTERN
#endif

EXTERN TABLE_T usertable;
void tables_init(void);

#undef EXTERN
#endif
