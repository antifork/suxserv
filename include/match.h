#ifndef __match_h__
#define __match_h__

#include "sux.h"

gint mycmp(const gchar *, const gchar *);
gint myncmp(const gchar *, const gchar *, gint);

gboolean __sux_hash_strcmp(gconstpointer, gconstpointer);
gboolean __sux_hash_strncmp(gconstpointer, gconstpointer, gint);

extern guchar __tolowertab[];
extern guchar __touppertab[];

#ifdef strcmp
#undef strcmp
#endif

#ifdef strncmp
#undef strncmp
#endif

#ifdef tolower
#undef tolower
#endif

#ifdef toupper
#undef toupper
#endif

#undef SUXSUX

#define strcmp		mycmp
#define strncmp		myncmp

#define tolower(x)	__tolowertab[(gint)(x)]
#define toupper(x)	__touppertab[(gint)(x)]

#endif
