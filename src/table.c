/*
 * Copyright 2002 Barnaba Marcello <vjt@azzurra.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2a. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 2b. Redistribution in binary form requires specific prior written
 *     authorization of the maintainer.
 * 
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *    This product includes software developed by Chip Norkus.
 * 
 * 4. The names of the maintainer, developers and contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE MAINTAINER, DEVELOPERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE DEVELOPERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sux.h"
#include "table.h"
#include "main.h"
#include "match.h"

/*
 * Fowler / Noll / Vo (FNV) Hash . .
 * Noll, Landon CUrt (LCN2)
 * chongo <was here> /\../\
 *
 * #define HASH(sta, end, hash)  while(end != sta) { hash = ((hash * 16777619UL) ^ (*end--)) }
 */
#define FNV_PRIME	16777619UL

G_INLINE_FUNC guint FNV_hash(guchar *s)
{
    register guint h = 0;
    guchar *e = s + strlen(s) - 1;

    while(e != s)
	h = ((h * FNV_PRIME) ^ tolower(*e--));

    return h;
}

/* Taner had BITS_PER_COL 4 BITS_PER_COL_MASK 0xF - Dianora */

#define BITS_PER_COL 3
#define BITS_PER_COL_MASK 0x7
#define MAX_SUB     (1<<BITS_PER_COL)

/* Client hash table 
 * used in hash.c 
 */

#define U_MAX_INITIAL  8192
#define U_MAX_INITIAL_MASK (U_MAX_INITIAL-1)
#define U_MAX (U_MAX_INITIAL*MAX_SUB)

/* Channel hash table 
 * used in hash.c 
 */

#define CH_MAX_INITIAL  2048
#define CH_MAX_INITIAL_MASK (CH_MAX_INITIAL-1)
#define CH_MAX (CH_MAX_INITIAL*MAX_SUB)

/*
 * stolen from bahamut/src/hash.c
 */
guint hash_nick_name(gchar *nname)
{
    register guint hash = 0;
    register gint hash2 = 0;
    register gchar lower;

    while (*nname)
    {
	lower = tolower(*nname);
	hash = (hash << 1) + lower;
	hash2 = (hash2 >> 1) + lower;
	nname++;
    }

    return ((hash & U_MAX_INITIAL_MASK) << BITS_PER_COL) +
	(hash2 & BITS_PER_COL_MASK);
}

/*
 * hash_channel_name
 * 
 * calculate a hash value on at most the first 30 characters of the
 * channel name. Most names are short than this or dissimilar in this
 * range. There is little or no point hashing on a full channel name
 * which maybe 255 chars long.
 */
gint hash_channel_name(guchar *hname)
{
    register guint hash = 0;
    register gint hash2 = 0;
    register gint i = 30;
    register gchar lower;

    while (*hname && --i)
    {
	lower = tolower(*hname);
	hash = (hash << 1) + lower;
	hash2 = (hash2 >> 1) + lower;
	hname++;
    }

    return ((hash & CH_MAX_INITIAL_MASK) << BITS_PER_COL) +
	(hash2 & BITS_PER_COL_MASK);
}

G_INLINE_FUNC void hash_tbl_dealloc(gchar *key, gpointer *data, GMemChunk *mem_pool)
{
    g_mem_chunk_free(mem_pool, data);
}

#define USER_PREALLOC		1024
#define CHANNEL_PREALLOC	1024
#define CMEMBERS_PREALLOC	CHANNEL_PREALLOC
#define SLINK_PREALLOC		(USER_PREALLOC * 4)

TABLE_DECLARE(user, User, hash_nick_name, nick, gchar);
TABLE_DECLARE(channel, Channel, hash_channel_name, chname, gchar);

MEMPOOL_DECLARE(cmembers);
MEMPOOL_DECLARE(links);
    
void setup_tables(void)
{
    TABLE_SETUP_FUNC(user, USER_PREALLOC);
    TABLE_SETUP_FUNC(channel, CHANNEL_PREALLOC);

    MEMPOOL_SETUP_FUNC(cmembers, ChanMember, CMEMBERS_PREALLOC);
    MEMPOOL_SETUP_FUNC(links, SLink, SLINK_PREALLOC);
}
