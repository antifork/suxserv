#ifndef __usertable_h__
#define __usertable_h__

#ifndef USERTABLE
# define EXTERN extern
#else
# define EXTERN
#endif

EXTERN TABLE_T usertable;
void tables_init(void);

#undef EXTERN
#endif
