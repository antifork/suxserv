/* insert a (C) notice here */

#define MAIN
#include "sux.h"
#include "dbuf.h"
#include "main.h"
#include "parse.h"
#include "usertable.h"

MyData me;

extern int errno;

static void io_loop();

void fatal(char *fmt, ...)
{
    va_list ap;
    int save_errno = errno;

    va_start (ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, ": %s\n", save_errno ? strerror(save_errno) : "Terminated.");
    exit(EXIT_FAILURE);
}

int connect_server(char *host, unsigned int port)
{
    struct sockaddr_in sock, my_addr;
    int ret, nb;
    socklen_t namelen = sizeof(my_addr);
    char hostbuf[HOSTLEN + 1];

    if(!inet_pton(AF_INET, host, (void*)&sock.sin_addr))
    {
	struct hostent *he = gethostbyname(host);
	if(!he)
	    fatal("gethostbyname()");
	memcpy(&sock.sin_addr, he->h_addr_list[0], he->h_length);
    }

    sock.sin_port = port;
    sock.sin_family = AF_INET;

    printf("connecting to %s ...", inet_ntop(AF_INET,
		(const void *) &sock.sin_addr, hostbuf, HOSTLEN-1));

    if((ret = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	fatal("socket()");

    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = 0;
    my_addr.sin_family = AF_INET;

    if(bind(ret, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) < 0)
	fatal("bind()");

    if(connect(ret, (struct sockaddr *)&sock, sizeof(struct sockaddr_in)) < 0)
	fatal("connect()");

    if(getsockname(ret, (struct sockaddr *)&my_addr, &namelen) < 0)
	fatal("getsockname()");

    if((nb = fcntl(ret, F_GETFL, 0)) < 0)
	fatal("fcntl(%d, F_GETFL, 0)", ret);
    else if(fcntl(ret, F_SETFL, nb | O_NONBLOCK) < 0)
	fatal("fcntl(%d, F_SETFL, nb | O_NONBLOCK)");

    printf(" connected.\n");
    return ret;
}

void dopacket(char *buffer, size_t len)
{
    static char buf[BUFSIZE << 1];
    static short count = 0;
    char *mybuf, *inbuf;

    mybuf = buf + count;
    inbuf = buffer;

    while(len--)
    {
	*mybuf = *inbuf++;
	if(*mybuf < '\16' && (*mybuf == '\r' || *mybuf == '\n'))
	{
	    if(*inbuf == '\n')
		continue;
	    *mybuf++ = ' ';
	    *mybuf = '\0';
	    parse(buf, mybuf);
	    mybuf = buf;
	    continue;
	}
	mybuf++;
    }
    count = mybuf - buf;
    return;
}

static __inline void io_error(char *func, int e)
{
    switch(e)
    {
	case EWOULDBLOCK:
	case EINTR:
	    break;
	case 0:
	    fatal("connection closed");
	    break;
	default:
	    fatal(func);
	    break;
    }
}

int send_out(char *fmt, ...)
{
    va_list ap;
    char buffer[BUFSIZE + 1];
    size_t len;
    
    va_start(ap, fmt);
    len = vsnprintf(buffer, BUFSIZE, fmt, ap);
    va_end(ap);

    if(len > BUFSIZE - 2)
    {
	buffer[BUFSIZE - 1] = '\n';
	buffer[BUFSIZE] = '\0';
	len = BUFSIZE;
    }
    else
    {
	buffer[len] = '\n';
	buffer[len+1] = '\0';
	len++;
    }
    if(!dbuf_put(&me.sendQ, buffer, len))
	fatal("dbuf_put()");

    return len;
}

static void io_loop()
{
    fd_set read_set, write_set;
    struct timeval wait;
    char readbuf[IOBUFSIZE + 1];
    char writebuf[IOBUFSIZE + 1];
    int count = 0;
    
    dbuf_init();
    send_out("PASS %s :TS", me.pass);
    send_out("CAPAB TS3 NOQUIT SSJOIN UNCONNECT NICKIP TSMODE");
    send_out("SVINFO 3 1 0 :%lu", time(NULL));
    send_out("SERVER %s 1 :%s", me.name, me.info);
    
    memset(readbuf, 0x0, IOBUFSIZE + 1);
    memset(writebuf, 0x0, IOBUFSIZE + 1);

    for(;;)
    {
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	wait.tv_sec = 5;
	wait.tv_usec = 0;

	FD_SET(me.sock, &read_set);
	if(DBufLength(&me.sendQ))
	{
	    FD_SET(me.sock, &write_set);
	}

	if(select(me.sock + 1, &read_set, &write_set, NULL, &wait) >= 0)
	{
	    if(FD_ISSET(me.sock, &read_set))
	    {
		while((count = read(me.sock, readbuf, IOBUFSIZE)) > 0)
		{
		    readbuf[count] = '\0';
		    if(!dbuf_put(&me.recvQ, readbuf, count))
			fatal("dbuf_put()");
		}
		if(count < 0)
		{
		    io_error("read()", errno);
		}
	    }
	    if(FD_ISSET(me.sock, &write_set))
	    {
		while(DBufLength(&me.sendQ))
		{
		    char *p = dbuf_map(&me.sendQ, &count);
		    if(count <= 0)
		    {
			break;
		    }
		    if(write(me.sock, p, count) < 0)
		    {
			io_error("write()", errno);
		    }
		    dbuf_delete(&me.sendQ, count);
		    
/*		    if((count = dbuf_get(&me.sendQ, writebuf, IOBUFSIZE)) <= 0)
			break;*/
//		    writebuf[count] = '\0';
//		    if(write(me.sock, writebuf, count) < 0)
//		    {
//			io_error("write()", errno);
//		    }
		} 
	    }

	    while(DBufLength(&me.recvQ))
	    {
		if((count = dbuf_get(&me.recvQ, readbuf, IOBUFSIZE)) == 0)
		    break;
		dopacket(readbuf, count);
	    }
	}
	else
	{
	    io_error("select()", errno);
	}
    }
}

void exit_func(int sig)
{
    fprintf(stderr, "received signal %d, quitting ..\n",
	    sig);
    exit(0);
}

int main(int argc, char **argv)
{
    strcpy(me.name, "services.dal.net");
    strcpy(me.info, "SuxServices 0.001");
    strcpy(me.pass, "codio");
    strcpy(me.host, "127.0.0.1");
    me.port = htons(6667);
    signal(SIGINT, exit_func);
    
    if((me.sock = connect_server(me.host, me.port)))
    {
	tables_init();
	io_loop();

	/* UNREACHABLE */
	return 0;
    }

    fatal("cannot connect to server %s:%d", me.host, me.port);

    return 0;
}
