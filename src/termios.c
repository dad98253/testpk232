/* Copyright (c) 2019, Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  // HAVE_CONFIG_H

#include "appf.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

//#include "com2net.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#define BIGBUFSIZE	10000

struct flag
{
	char name[16];
	unsigned long flag;
};

char *progname = "termios";

/* Keep the entries sorted alphabetically. */
#define V(name) { #name, name }
struct flag c_iflag[] =
{
	  V(BRKINT)
	, V(ICRNL)
	, V(IGNBRK)
	, V(IGNCR)
	, V(IGNPAR)
#ifdef IMAXBEL
	, V(IMAXBEL)
#endif
	, V(INLCR)
	, V(ISTRIP)
#ifdef IUTF8
	, V(IUTF8)
#endif
	, V(IXANY)
	, V(IXOFF)
	, V(IXON)
	, V(PARMRK)
};

struct flag c_oflag[] =
{
	  V(BS1)
	, V(NL1)
	, V(ONLCR)
#ifdef ONOEOT
	, V(ONOEOT)
#endif
	, V(OPOST)
#ifdef OXTABS
	, V(OXTABS)
#endif
};

struct flag c_cflag[] =
{
	  V(CLOCAL)
	, V(CREAD)
	, V(CSIZE)
	, V(CSTOPB)
	, V(HUPCL)
};

struct flag c_lflag[] =
{
	  V(ECHO)
	, V(ECHOCTL)
	, V(ECHOE)
	, V(ECHOK)
	, V(ECHOKE)
	, V(ECHONL)
	, V(ECHOPRT)
	, V(FLUSHO)
	, V(ICANON)
	, V(IEXTEN)
	, V(ISIG)
	, V(NOFLSH)
	, V(PENDIN)
	, V(TOSTOP)
#ifdef XCASE
	, V(XCASE)
#endif
};

struct flag speeds[] =
{
	  V(B50)
	, V(B75)
	, V(B110)
	, V(B134)
	, V(B150)
	, V(B200)
	, V(B300)
	, V(B600)
	, V(B1200)
	, V(B1800)
	, V(B2400)
	, V(B4800)
	, V(B9600)
#ifdef B57600
	, V(B57600)
#endif
#ifdef B115200
	, V(B115200)
#endif
	, V(B19200)
#ifdef B230400
	, V(B230400)
#endif
	, V(B38400)
#ifdef B460800
	, V(B460800)
#endif
#ifdef B500000
	, V(B500000)
#endif
#ifdef B57600
	, V(B57600)
#endif
#ifdef B921600
	, V(B921600)
#endif
#ifdef B1000000
	, V(B1000000)
#endif
#ifdef B1152000
	, V(B1152000)
#endif
#ifdef B1500000
	, V(B1500000)
#endif
#ifdef B2000000
	, V(B2000000)
#endif
#ifdef B2500000
	, V(B2500000)
#endif
#ifdef B3000000
	, V(B3000000)
#endif
#ifdef B3500000
	, V(B3500000)
#endif
#ifdef B4000000
	, V(B4000000)
#endif
};

struct flag c_cc[] =
{
	  V(VDISCARD)
#ifdef VDSUSP
	, V(VDSUSP)
#endif
	, V(VEOF)
	, V(VEOL)
	, V(VEOL2)
	, V(VERASE)
	, V(VINTR)
	, V(VKILL)
	, V(VLNEXT)
	, V(VMIN)
	, V(VQUIT)
	, V(VREPRINT)
	, V(VSTART)
#ifdef VSTATUS
	, V(VSTATUS)
#endif
	, V(VSTOP)
	, V(VSUSP)
#ifdef VSWTCH
	, V(VSWTCH)
#endif
	, V(VTIME)
	, V(VWERASE)
	, V(_POSIX_VDISABLE)
};


struct flag serial_lines[] =
{
  V(TIOCM_LE)
, V(TIOCM_DTR)
, V(TIOCM_RTS)
, V(TIOCM_ST)
, V(TIOCM_SR)
, V(TIOCM_CTS)
, V(TIOCM_CAR)
, V(TIOCM_CD)
, V(TIOCM_RNG)
, V(TIOCM_RI)
, V(TIOCM_DSR)
};

#undef V


int flag2speed( int sp )
{
	switch (sp)
	{
#ifdef B0
	case B0:
		return 0;
		break;
#endif
#ifdef B50
	case B50:
		return 50;
		break;
#endif
#ifdef B75
	case B75:
		return 75;
		break;
#endif
#ifdef B110
	case B110:
		return 110;
		break;
#endif
#ifdef B134
	case B134:
		return 134;
		break;
#endif
#ifdef B150
	case B150:
		return 150;
		break;
#endif
#ifdef B200
	case B200:
		return 200;
		break;
#endif
#ifdef B300
	case B300:
		return 300;
		break;
#endif
#ifdef B600
	case B600:
		return 600;
		break;
#endif
#ifdef B1200
	case B1200:
		return 1200;
		break;
#endif
#ifdef B1800
	case B1800:
		return 1800;
		break;
#endif
#ifdef B2400
	case B2400:
		return 2400;
		break;
#endif
#ifdef B4800
	case B4800:
		return 4800;
		break;
#endif
#ifdef B9600
	case B9600:
		return 9600;
		break;
#endif
#ifdef B19200
	case B19200:
		return 19200;
		break;
#endif
#ifdef B38400
	case B38400:
		return 38400;
		break;
#endif
#ifdef B57600
	case B57600:
		return 57600;
		break;
#endif
#ifdef B115200
	case B115200:
		return 115200;
		break;
#endif
#ifdef B230400
	case B230400:
		return 230400;
		break;
#endif
#ifdef B460800
	case B460800:
		return 460800;
		break;
#endif
#ifdef B500000
	case B500000:
		return 500000;
		break;
#endif
#ifdef B576000
	case B576000:
		return 576000;
		break;
#endif
#ifdef B921600
	case B921600:
		return 921600;
		break;
#endif
#ifdef B1000000
	case B1000000:
		return 1000000;
		break;
#endif
#ifdef B1152000
	case B1152000:
		return 1152000;
		break;
#endif
#ifdef B1500000
	case B1500000:
		return 1500000;
		break;
#endif
#ifdef B2000000
	case B2000000:
		return 2000000;
		break;
#endif
#ifdef B2500000
	case B2500000:
		return 2500000;
		break;
#endif
#ifdef B3000000
	case B3000000:
		return 3000000;
		break;
#endif
#ifdef B3500000
	case B3500000:
		return 3500000;
		break;
#endif
#ifdef B4000000
	case B4000000:
		return 4000000;
		break;
#endif
	default:
		return -1;
	}
}

int speed2flag( int sp )
{
	switch (sp)
	{
#ifdef B0
	case 0:
		return B0;
		break;
#endif
#ifdef B50
	case 50:
		return B50;
		break;
#endif
#ifdef B75
	case 75:
		return B75;
		break;
#endif
#ifdef B110
	case 110:
		return B110;
		break;
#endif
#ifdef B134
	case 134:
		return B134;
		break;
#endif
#ifdef B150
	case 150:
		return B150;
		break;
#endif
#ifdef B200
	case 200:
		return B200;
		break;
#endif
#ifdef B300
	case 300:
		return B300;
		break;
#endif
#ifdef B600
	case 600:
		return B600;
		break;
#endif
#ifdef B1200
	case 1200:
		return B1200;
		break;
#endif
#ifdef B1800
	case 1800:
		return B1800;
		break;
#endif
#ifdef B2400
	case 2400:
		return B2400;
		break;
#endif
#ifdef B4800
	case 4800:
		return B4800;
		break;
#endif
#ifdef B9600
	case 9600:
		return B9600;
		break;
#endif
#ifdef B19200
	case 19200:
		return B19200;
		break;
#endif
#ifdef B38400
	case 38400:
		return B38400;
		break;
#endif
#ifdef B57600
	case 57600:
		return B57600;
		break;
#endif
#ifdef B115200
	case 115200:
		return B115200;
		break;
#endif
#ifdef B230400
	case 230400:
		return B230400;
		break;
#endif
#ifdef B460800
	case 460800:
		return B460800;
		break;
#endif
#ifdef B500000
	case 500000:
		return B500000;
		break;
#endif
#ifdef B576000
	case 576000:
		return B576000;
		break;
#endif
#ifdef B921600
	case 921600:
		return B921600;
		break;
#endif
#ifdef B1000000
	case 1000000:
		return B1000000;
		break;
#endif
#ifdef B1152000
	case 1152000:
		return B1152000;
		break;
#endif
#ifdef B1500000
	case 1500000:
		return B1500000;
		break;
#endif
#ifdef B2000000
	case 2000000:
		return B2000000;
		break;
#endif
#ifdef B2500000
	case 2500000:
		return B2500000;
		break;
#endif
#ifdef B3000000
	case 3000000:
		return B3000000;
		break;
#endif
#ifdef B3500000
	case 3500000:
		return B3500000;
		break;
#endif
#ifdef B4000000
	case 4000000:
		return B4000000;
		break;
#endif
	default:
		return -1;
	}
}

void
die(char *fmt, ...)
{
	va_list ap;
	int err;

	err = errno;
	fprintf(stderr, "%s: ", progname);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", strerror(err));
	fflush(stderr);
	exit(1);
}

void
printspeed(unsigned long flag, int fst, int speedonly)
{
	struct flag *p;

	for (p = speeds; p < speeds + ARRAY_SIZE(speeds); p++)
		if (flag == p->flag)
		{
			printf("%s%s", &"|"[fst], &p->name[speedonly]);
			return;
		}

	printf("%s%lu", &"|"[fst], flag);
}
int sprintspeed(char *buf,unsigned long flag, int fst, int speedonly)
{
	struct flag *p;
	char *ps;
	int loc=0;

	ps=buf;
	for (p = speeds; p < speeds + ARRAY_SIZE(speeds); p++)
		if (flag == p->flag)
		{
			loc+=sprintf(ps+loc,"%s%s", &"|"[fst], &p->name[speedonly]);
			return(loc);
		}

	loc+=sprintf(ps+loc,"%s%lu", &"|"[fst], flag);
	return(loc);
}

void
print(char *name, unsigned long flag, struct flag flags[], unsigned nflags)
{
	struct flag *p;
	int fst;

//	printf("%s\t\t", name);
	printf("%s        ", name);

	fst = 1;
	for (p = flags; p < flags + nflags; p++)
		if (flag & p->flag)
		{
			printf("%s%s", &"|"[fst], p->name);
			flag -= p->flag;
			fst = 0;
		}

	if (flags == c_cflag)
		if (flag)
			printspeed(flag, fst, /* speedonly */ 0);

	printf("\n");
}

int sprint(char * buf, char *name, unsigned long flag, struct flag flags[], unsigned nflags)
{
	struct flag *p;
	int fst;
	char *ps;
	int loc=0;

	ps=buf;
//	loc+=sprintf(ps,"%s\t\t", name);
	loc+=sprintf(ps,"%s        ", name);

	fst = 1;
	for (p = flags; p < flags + nflags; p++)
		if (flag & p->flag)
		{
			loc+=sprintf(ps+loc,"%s%s", &"|"[fst], p->name);
			fst = 0;
		}

	if (flags == c_cflag)
		{
		       if( (flag&CSIZE) == CS5)loc+=sprintf(ps+loc,"|CS5");
		       if( (flag&CSIZE) == CS6)loc+=sprintf(ps+loc,"|CS6");
		       if( (flag&CSIZE) == CS7)loc+=sprintf(ps+loc,"|CS7");
		       if( (flag&CSIZE) == CS8)loc+=sprintf(ps+loc,"|CS8");
		}
//			loc+=sprintspeed(ps+loc,flag, fst, /* speedonly */ 0);

	return(loc);
}


int scprint(char * buf, char *name, unsigned char * ccs, struct flag flags[], unsigned nflags)
{
	struct flag *p;
	int fst;
	char *ps;
	int loc=0;
	int i;

	ps=buf;
//	loc+=sprintf(ps,"%s\t\t", name);
	loc+=sprintf(ps,"%s        ", name);

	fst = 1;
	for (p = flags, i = 0; p < flags + nflags; p++, i++)
		if ( *(ccs+(p->flag)) != _POSIX_VDISABLE )
		{
			loc+=sprintf(ps+loc,"%s%s=%02xx", &","[fst], p->name, *(ccs+(p->flag)));
			fst = 0;
		}

	return(loc);
}
/*
int
main(int argc, char **argv)
{
*/
int termios( int fd) {
	struct termios t;
	cc_t *cc;
	char buf[BIGBUFSIZE];
	int loc = 0;
/*	char *p, *q;
	int fd;

	progname = argv[0];

	fd = 0;
	if (argc > 1)
	{
		p = q = argv[1];
		fd = (int) strtol(p, &q, 10);

		if (*q != '\0')
			fd = open(p, O_RDONLY);

		if (fd == -1)
			die("open(%s)", p);
	}
*/
	if (tcgetattr(fd, &t))
		die("tcgetattr");
	printf("4 tty.c_cflag & CSIZE = %04xx\n", (t.c_cflag)&CSIZE);
//#define V(name) print(#name, t.name, name, ARRAY_SIZE(name))
//	V(c_iflag);
//	V(c_oflag);
//	V(c_cflag);
//	V(c_lflag);
//#undef V
#define V(name) sprint(buf, #name, t.name, name, ARRAY_SIZE(name))
	V(c_iflag); af_log_print(LOG_DEBUG, "%s",buf);
	V(c_oflag); af_log_print(LOG_DEBUG, "%s",buf);
	V(c_cflag); af_log_print(LOG_DEBUG, "%s",buf);
	V(c_lflag); af_log_print(LOG_DEBUG, "%s",buf);
#undef V

//	printf("c_cc\t\t");
	for (cc = t.c_cc; cc < t.c_cc + ARRAY_SIZE(t.c_cc); cc++)
//		printf("%s%u", &","[cc == t.c_cc], *cc);
		loc+=sprintf(buf+loc,"%s%u", &","[cc == t.c_cc], *cc);
//	printf("\n");
	af_log_print(LOG_DEBUG, "terminal special characters: %s",buf);
#define V(name) scprint(buf, #name, t.name, name, ARRAY_SIZE(name))
	V(c_cc);  af_log_print(LOG_DEBUG, "%s",buf);
#undef V

	af_log_print(LOG_DEBUG, "tty.c_cc[VTIME] = %d\n",t.c_cc[VTIME]);
	af_log_print(LOG_DEBUG, "tty.c_cc[VMIN] = %d\n",t.c_cc[VMIN]);

	// Not all platforms support c_ispeed and c_ospeed.
#ifndef _AIX
//	printf("c_ispeed\t"); printspeed(cfgetispeed(&t), 1, 1); printf("\n");
//	printf("c_ospeed\t"); printspeed(cfgetospeed(&t), 1, 1); printf("\n");
//	printf("\n");
	sprintf(buf," %i",flag2speed(cfgetispeed(&t)));
	af_log_print(LOG_DEBUG, "c_ispeed  %s",buf);
	sprintf(buf," %i",flag2speed(cfgetospeed(&t)));
//	printf("\n");
	af_log_print(LOG_DEBUG, "c_ospeed  %s",buf);
#endif

	return 0;
}


//Check the condition of DTR on the serial port.
int checkserial(int fd)
{
	char buf[BIGBUFSIZE];
    int serial;
//   TIOCM_LE    DSR (data set ready/line enable)
//   TIOCM_DTR   DTR (data terminal ready)
//   TIOCM_RTS   RTS (request to send)
//   TIOCM_ST    Secondary TXD (transmit)
//   TIOCM_SR    Secondary RXD (receive)
//   TIOCM_CTS   CTS (clear to send)
//   TIOCM_CAR   DCD (data carrier detect)
//   TIOCM_CD    see TIOCM_CAR
//   TIOCM_RNG   RNG (ring)
//   TIOCM_RI    see TIOCM_RNG
//   TIOCM_DSR   DSR (data set ready)
    ioctl(fd, TIOCMGET, &serial);
#define V(name) sprint(buf, #name, serial, name, ARRAY_SIZE(name))
	V(serial_lines); af_log_print(LOG_DEBUG, "%s",buf);
#undef V
	return (0);
}

