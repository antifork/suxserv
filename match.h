#ifndef __match_h__
#define __match_h__

int mycmp(const char *, const char *);
int myncmp(const char *, const char *, int);

#ifdef strcmp
#undef strcmp
#endif
#define strcmp mycmp

#endif
