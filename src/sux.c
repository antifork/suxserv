[...]

#include <glib.h>

#define FINGER_IN_THE_ASS 0x1
#define SUX 0x2

guint girls, man, coding;
guint64 happyness;

void girls_sux(void)
{
    while(TRUE)
    {
	girls |= FINGER_IN_THE_ASS;
    }
}

void man_with_girl(void)
{
    while(G_LIKELY(girls & FINGER_IN_THE_ASS)) // almost unuseful
    {
	man |= SUX;
    }
}

void man_life(void)
{
    while(coding && happyness)
    {
	
	if(man & SUX)
	{
	    coding--;
	    happyness >> 1;
	}
	else
	{
	    coding++;
	    happyness << 1;
	}
    }

    g_thread_exit(NULL);
}

gint main(void)
{
    GThread *man_life_thr;

    g_thread_create((GThreadFunc)girls_sux, FALSE, NULL);
    g_thread_create((GThreadFunc)man_with_girl, FALSE, NULL);
    man_life_ths = g_thread_create((GThreadFunc)man_life, TRUE, NULL);

    g_thread_join(man_life_thr);

    abort();
}

[...]
