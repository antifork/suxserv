[...]

#include <glib.h>

#define FINGER_IN_THE_ASS 0x1
#define SOMATIZZA 0x2


guint girls, man, autismo;
guint64 happyness;

while(TRUE)
{
    girls |= FINGER_IN_THE_ASS;
}

while(G_LIKELY(girls & FINGER_IN_THE_ASS)) // almost unuseful
{
    man |= SOMATIZZA;
}

if(man & SOMATIZZA)
{
    autismo--;
}
else
{
    autismo++;
    happyness << 1;
}

[...]
