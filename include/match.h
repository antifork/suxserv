#ifndef __match_h__
#define __match_h__

#include "sux.h"

int mycmp(const gchar *, const gchar *);
int myncmp(const gchar *, const gchar *, int);

#ifdef strcmp
#undef strcmp
#endif
#define strcmp mycmp

#endif
