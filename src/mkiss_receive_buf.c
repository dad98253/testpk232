/*
 * mkiss_receive_buf.c
 *
 *  Created on: May 15, 2025
 *      Author: dad
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mkiss.h"				// the ax struct definition
#include "appf.h"

/* some arch define END as assembly function ending, just undef it */
#undef  END
/* SLIP/KISS protocol characters. */
#define END             0300            /* indicates end of frame       */
#define ESC             0333            /* indicates byte stuffing      */
#define ESC_END         0334            /* ESC ESC_END means END 'data' */
#define ESC_ESC         0335            /* ESC ESC_ESC means ESC 'data' */

struct mkiss axstruct;
struct mkiss *ax = &axstruct;

extern af_daemon_t mydaemon;
extern int setraw;

static void kiss_unesc(struct mkiss *ax, unsigned char s);
static void ax_bump(struct mkiss *ax);
static int test_and_clear_bit (const long unsigned int x , long unsigned int * y);

/*
 * Handle the 'receiver data ready' interrupt.
 * This function is called by the 'tty_io' module in the kernel when
 * a block of data has been received, which can now be decapsulated
 * and sent on to the AX.25 layer for further processing.
 *
 * receive_buf
 *
 * [DRV] void ()(struct tty_struct *tty, const u8 *cp, const u8 *fp, size_t count)
 *
 * This function is called by the low-level tty driver to send characters received by the
 * hardware to the line discpline for processing. cp is a pointer to the buffer of input
 * character received by the device. fp is a pointer to an array of flag bytes which
 * indicate whether a character was received with a parity error, etc. fp may be NULL to
 * indicate all data received is TTY_NORMAL.
 *
 */
void mkiss_receive_buf(struct tty_struct *tty, const char *cp, const char *fp, int count)
{
//	struct mkiss *ax = mkiss_get(tty);

//	if (!ax)
//		return;

	/*
	 * Argh! mtu change time! - costs us the packet part received
	 * at the change
	 */
//	if (ax->mtu != ax->dev->mtu + 73)
//		ax_changedmtu(ax);

	/* Read the characters out of the buffer */
	while (count--) {
//		if (fp != NULL && *fp++) {
//			if (!test_and_set_bit(AXF_ERROR, &ax->flags))
//				ax->dev->stats.rx_errors++;
//			cp++;
//			continue;
//		}

		kiss_unesc(ax, *cp++);
	}

//	mkiss_put(ax);
//	tty_unthrottle(tty);
}


static void kiss_unesc(struct mkiss *ax, unsigned char s)
{
	if ( mydaemon.log_level > LOG_WARNING ) printf("X");
	if ( setraw ) {
		if ( s == END ) {
			if (ax->rcount > 2) {
				ax_bump(ax);
				ax->rcount = 0;
			}
		}
	} else {
		switch (s) {
		case END:
			if ( mydaemon.log_level > LOG_WARNING ) printf("END,test_and_clear_bit = %i, rcount = %i, buffsize = %i\n",!test_and_clear_bit(AXF_ERROR, &ax->flags),ax->rcount,ax->buffsize);
			/* drop keeptest bit = VSV */
			if (test_bit(AXF_KEEPTEST, &ax->flags))
				clear_bit(AXF_KEEPTEST, &ax->flags);

			if (!test_and_clear_bit(AXF_ERROR, &ax->flags) && (ax->rcount > 2)) {
				ax_bump(ax);
			}

			clear_bit(AXF_ESCAPE, &ax->flags);
			ax->rcount = 0;
			return;

		case ESC:
			set_bit(AXF_ESCAPE, &ax->flags);
			return;
		case ESC_ESC:
			if (test_and_clear_bit(AXF_ESCAPE, &ax->flags))
				s = ESC;
			break;
		case ESC_END:
			if (test_and_clear_bit(AXF_ESCAPE, &ax->flags))
				s = END;
			break;
		}
	}

//	spin_lock_bh(&ax->buflock);
	if (!test_bit(AXF_ERROR, &ax->flags)) {
		if (ax->rcount < ax->buffsize) {
			ax->rbuff[ax->rcount++] = s;
//			spin_unlock_bh(&ax->buflock);
			return;
		} else {
			printf("AXF_ERROR");
		}

//		ax->dev->stats.rx_over_errors++;
		set_bit(AXF_ERROR, &ax->flags);
	}
//	spin_unlock_bh(&ax->buflock);
}


/* Send one completely decapsulated AX.25 packet to the AX.25 layer. */
static void ax_bump(struct mkiss *ax)
{
//	struct sk_buff *skb;
//	int count;

//	spin_lock_bh(&ax->buflock);
	if ( !setraw ) {
		if (ax->rbuff[0] > 0x0f) {
			if (ax->rbuff[0] & 0x80) {
				if ( mydaemon.log_level > LOG_WARNING ) fprintf(stderr,"rbuff[0] & 0x80 true\n");
				ax->rcount -= 2;
				*ax->rbuff &= ~0x80;
			} else if (ax->rbuff[0] & 0x20)  {
				if ( mydaemon.log_level > LOG_WARNING ) fprintf(stderr,"rbuff[0] & 0x20 true\n");
				ax->rcount -= 2;

				/*
				 * dl9sau bugfix: the trailling two bytes flexnet crc
				 * will not be passed to the kernel. thus we have to
				 * correct the kissparm signature, because it indicates
				 * a crc but there's none
				 */
				*ax->rbuff &= ~0x20;
			}
		}
	}

	ax->bump = ax->count = ax->rcount;

    memcpy(ax->myrbuff, ax->rbuff, (size_t)ax->count);


}

static int test_and_clear_bit (const long unsigned int x , long unsigned int * y) {
	int iret = 0;
	if ( mydaemon.log_level > LOG_WARNING ) printf("x,y in = %lx,%lx; ",x,*y);
	iret = test_bit(x , y);
	clear_bit(x , y);
	if ( mydaemon.log_level > LOG_WARNING ) printf("x,y out = %lx,%lx; iret=%i\n",x,*y,iret);
	return iret;
}


/* Open the low-level part of the AX25 channel. Easy! */
int ax_open()
{
//	struct mkiss *ax = netdev_priv(dev);
	unsigned long len;

//	if (ax->tty == NULL)
//		return -3;

	/*
	 * Allocate the frame buffers:
	 *
	 * rbuff	Receive buffer.
	 * xbuff	Transmit buffer.
	 */
	len = 2000; //	len = dev->mtu * 2;

	/*
	 * allow for arrival of larger UDP packets, even if we say not to
	 * also fixes a bug in which SunOS sends 512-byte packets even with
	 * an MSS of 128
	 */
	if (len < 576 * 2)
		len = 576 * 2;

	if ((ax->rbuff = malloc(len + 4)) == NULL)
		goto norbuff;

	if ((ax->myrbuff = malloc(len + 4)) == NULL)
		goto norbuff;

	if ((ax->xbuff = malloc(len + 4)) == NULL)
		goto noxbuff;

	ax->mtu	     = 1000 + 73;
	ax->buffsize = len;
	ax->rcount   = 0;
	ax->xleft    = 0;

	ax->flags   &= (1 << AXF_INUSE);      /* Clear ESCAPE & ERROR flags */

//	spin_lock_init(&ax->buflock);

	return 0;

noxbuff:
	free(ax->rbuff);

norbuff:
	return -4;
}

