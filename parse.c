/* C code produced by gperf version 2.7.2 */
/* Command-line: gperf -tDW cmds -N hash_get_cmd -K cmd parse.gperf  */
#include "h.h"
#include "services.h"
#include "main.h"
#include "parse.h"
#include "match.h"

static char *para[MAXPARA + 1];
static char sender[HOSTLEN + 1];
/* XXX: LATAH
static void remove_unknown(char *, char *);
*/
extern MyData me;
extern User *find_client(char *);
struct Message { char *cmd; int (*func)(int, char **); unsigned int count; };

#define TOTAL_KEYWORDS 30
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 7
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 64
/* maximum key range = 63, duplicates = 1 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static unsigned char asso_values[] =
    {
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65,  0, 65,  0, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65,  5, 10, 20, 20, 25,
      65,  8, 65, 10, 15, 30, 20, 18, 25,  0,
       0, 10, 23,  0,  3, 65,  0, 65, 65,  0,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#endif
struct Message *
hash_get_cmd (str, len)
     register const char *str;
     register unsigned int len;
{
  static struct Message cmds[] =
    {
      {"OS", m_os, 0},
      {"436", m_nickcoll, 0},
      {"PASS", m_pass, 0},
      {"STATS", m_stats, 0},
      {"SVINFO", m_svinfo, 0},
      {"PART", m_part, 0},
      {"SQUIT", m_squit, 0},
      {"AWAY", m_away, 0},
      {"PING", m_ping, 0},
      {"PONG", m_pong, 0},
      {"INFO", m_info, 0},
      {"PRIVMSG", m_private, 0},
      {"QUIT", m_quit, 0},
      {"BURST", m_burst, 0},
      {"MS", m_cs, 0},
      {"CS", m_rs, 0},
      {"RS", m_rs, 0},
      {"NS", m_ns, 0},
      {"TOPIC", m_topic, 0},
      {"SERVER", m_server, 0},
      {"SJOIN", m_sjoin, 0},
      {"VERSION", m_version, 0},
      {"CAPAB", m_capab, 0},
      {"MOTD", m_motd, 0},
      {"JOIN", m_join, 0},
      {"MODE", m_mode, 0},
      {"KILL", m_kill, 0},
      {"NOTICE", m_notice, 0},
      {"NICK", m_nick, 0},
      {"KICK", m_kick, 0}
    };

  static short lookup[] =
    {
       -1,  -1,   0,   1,   2,   3,   4,   5,   6,   7,
       -1,  -1, -54,  -1,  10,  11,  -1,  12,  13,  -1,
       14,  -1,  15, -22,  -2,  16,  -1,  17,  18,  19,
       20,  -1,  21,  -1,  -1,  22,  -1,  -1,  -1,  -1,
       -1,  -1,  23,  -1,  24,  -1,  -1,  25,  -1,  -1,
       -1,  -1,  -1,  -1,  26,  -1,  27,  -1,  -1,  28,
       -1,  -1,  -1,  -1,  29
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = cmds[index].cmd;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &cmds[index];
            }
          else if (index < -TOTAL_KEYWORDS)
            {
              register int offset = - 1 - TOTAL_KEYWORDS - index;
              register struct Message *wordptr = &cmds[TOTAL_KEYWORDS + lookup[offset]];
              register struct Message *wordendptr = wordptr + -lookup[offset + 1];

              while (wordptr < wordendptr)
                {
                  register const char *s = wordptr->cmd;

                  if (*str == *s && !strcmp (str + 1, s + 1))
                    return wordptr;
                  wordptr++;
                }
            }
        }
    }
  return 0;
}
/*
 * parse a buffer.
 * 
 * NOTE: parse() should not be called recusively by any other functions!
 */

int parse(char *buffer, char *bufend)
{
    char *ch, *s;
    int i;
    struct Message *mptr;

    fprintf(stderr, "parse(): %s\n", buffer);
    s = sender;
    *s = '\0';
    
    for (ch = buffer; *ch == ' '; ch++);	/* skip spaces */
    
    para[0] = me.uplink;
    if (*ch == ':') 
    {
	/*
	 * Copy the prefix to 'sender' assuming it terminates with
	 * SPACE (or NULL, which is an error, though).
	 */
	
	for (++ch; *ch && *ch != ' '; ++ch)
	    if (s < (sender + HOSTLEN))
		*s++ = *ch;		
	*s = '\0';
	
	/*
	 * Actually, only messages coming from servers can have the
	 * prefix--prefix silently ignored, if coming from a user
	 * client...
	 * 
	 * ...sigh, the current release "v2.2PL1" generates also null
	 * prefixes, at least to NOTIFY messages (e.g. it puts
	 * "sptr->nickname" as prefix from server structures where it's
	 * null--the following will handle this case as "no prefix" at
	 * all --msa  (": NOTICE nick ...")
	 */
	
	if (*sender)
	{
	    /* XXX: LATAH
	    User *from = find_client(sender);
	    */

	    /*
	     * okay, this doesn't seem to do much here.
	     * from->name _MUST_ be equal to sender. 
	     * That's what find_client does.
	     * find_client will find servers too, and since we don't use server
	     * masking, the find server call is useless (and very wasteful).
	     * now, there HAS to be a from and from->name and
	     * sender have to be the same
	     * for us to get to the next if. but the next if
	     * starts out with if(!from)
	     * so this is UNREACHABLE CODE! AGH! - lucas
	     *
	     *  if (!from || mycmp(from->name, sender))
	     *     from = find_server(sender, (aClient *) NULL);
	     *  else if (!from && strchr(sender, '@'))
	     *     from = find_nickserv(sender, (aClient *) NULL);
	     */

	    para[0] = sender;
	    /*
	     * Hmm! If the client corresponding to the prefix is not
	     * found--what is the correct action??? Now, I will ignore the
	     * message (old IRC just let it through as if the prefix just
	     * wasn't there...) --msa
	     */
	    /* XXX: LATAH
	    if (!from) 
	    {
		remove_unknown(sender, buffer);
		return -1;
	    }
	    */
	}
	while (*ch == ' ')
	    ch++;
    }

    if (*ch == '\0') 
	return (-1);
    /*
     * Extract the command code from the packet.  Point s to the end
     * of the command code and calculate the length using pointer
     * arithmetic.  Note: only need length for numerics and *all*
     * numerics must have parameters and thus a space after the command
     * code. -avalon
     * 
     * ummm???? - Dianora
     */

    s = strchr(ch, ' ');
    
    if (s)
        *s = '\0';
    
    mptr = hash_get_cmd(ch, s++ - ch);
    
    if (!mptr || !mptr->cmd) 
    {
	fprintf(stderr, "cmd not found (%s) ..\n", ch);
        return -1;
    }
    
    /*
     * Must the following loop really be so devious? On surface it
     * splits the message to parameters from blank spaces. But, if
     * paramcount has been reached, the rest of the message goes into
     * this last parameter (about same effect as ":" has...) --msa
     */

    /* Note initially true: s==NULL || *(s-1) == '\0' !! */
    
    i = 1;
    if (s) 
    {
	for (;;) 
	{
	    while (*s == ' ')
		*s++ = '\0';
	    
	    if (*s == '\0')
		break;
	    if (*s == ':') 
	    {
		/* The rest is a single parameter */
		para[i++] = s + 1;
		break;
	    }
	    para[i++] = s;
	    if (i >= MAXPARA)
		break;
	    
	    while(*s && *s != ' ')
		s++;
	}
    }
    
    para[i] = NULL;
    mptr->count++;
    
    return (*mptr->func) (i, para);
}

/* XXX: LATAH
static void remove_unknown(char *sender, char *buffer)
{
    / *
     * Do kill if it came from a server because it means there is a
     * ghost user on the other server which needs to be removed. -avalon
     * Tell opers about this. -Taner
     * /
    if (!strchr(sender, '.'))
	send_out(":%s KILL %s :%s (%s(?)",
		me.name, sender, me.name, sender);
    else
    {
	send_out(":%s GLOBOPS :Unknown prefix (%s), Squitting %s",
		me.name, buffer, sender);
	send_out(":%s SQUIT %s :Unknown prefix (%s)",
		me.name, sender, buffer);
    }
}
*/
