/*
 * mkiss.h
 *
 *  Created on: May 15, 2025
 *      Author: dad
 */

#ifndef MKISS_H_
#define MKISS_H_

struct mkiss {
	struct tty_struct	*tty;	/* ptr to TTY structure		*/
	struct net_device	*dev;	/* easy for intr handling	*/

	/* These are pointers to the malloc()ed frame buffers. */
//	spinlock_t		buflock;/* lock for rbuf and xbuf */
	unsigned char		*rbuff;	/* receiver buffer		*/
	int			rcount;	/* received chars counter       */
	unsigned char		*xbuff;	/* transmitter buffer		*/
	unsigned char		*xhead;	/* pointer to next byte to XMIT */
	int			xleft;	/* bytes left in XMIT queue     */

	/* Detailed SLIP statistics. */
	int		mtu;		/* Our mtu (to spot changes!)   */
	int		buffsize;	/* Max buffers sizes            */

	unsigned long	flags;		/* Flag values/ mode etc	*/
					/* long req'd: used by set_bit --RR */
#define AXF_INUSE	0		/* Channel in use               */
#define AXF_ESCAPE	1               /* ESC received                 */
#define AXF_ERROR	2               /* Parity, etc. error           */
#define AXF_KEEPTEST	3		/* Keepalive test flag		*/
#define AXF_OUTWAIT	4		/* is outpacket was flag	*/

	int		mode;
        int		crcmode;	/* MW: for FlexNet, SMACK etc.  */
	int		crcauto;	/* CRC auto mode */

#define CRC_MODE_NONE		0
#define CRC_MODE_FLEX		1
#define CRC_MODE_SMACK		2
#define CRC_MODE_FLEX_TEST	3
#define CRC_MODE_SMACK_TEST	4

//	refcount_t		refcnt;
//	struct completion	dead;
	int bump;	// added by jck
	int count;	// added by jck
	unsigned char		*myrbuff;	/* my receiver buffer		*/
};

#define test_bit(x , y) ( ( ( (x) & (*y) ) == 0 ) ? (0) : (1) )
#define	clear_bit(x , y) ( (*y) = ((*y) & (~x))  )
#define	set_bit(x , y) ( (*y) = ((*y) | (x))  )



#endif /* MKISS_H_ */
