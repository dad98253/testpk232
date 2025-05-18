/*
 ============================================================================
 Name        : testpk232.c
 Author      : dad
 Version     :
 Copyright   : dwtfywwi
 Description : test the Packrat 232 kiss interface in C, Ansi-style
 ============================================================================
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  // HAVE_CONFIG_H

#define _POSIX_SOURCE 1

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>

#include "appf.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
//#include <asm/termbits.h>
#include <sys/ioctl.h>
#include "panic.h"                /* Defines the PANIC macro */
#include "mkiss.h"				// the ax struct definition

#define BUFFSIZE 256

static int chan = -1;

#define VER_MAJOR 1
#define VER_MINOR 0

extern struct mkiss *ax;

af_daemon_t mydaemon;
int setkiss = 0;
int interpretkiss = 0;
int printnum = 1;

extern int termios( int fd);
extern void mkiss_receive_buf(struct tty_struct *tty, const char *cp, const char *fp, int count);
extern int checkserial(int fd);
extern int ax_open();
int trace (char * buff, int size);
int unpacket (char * buff, int size);
int checkPID(unsigned char PID);

void testpk232_usage( )
{
	printf("testpk232 [opts]\n" );
	printf("opts:\n" );
	printf(" -v    = Version\n" );
	printf(" -f    = run in foreground\n" );
	printf(" -s    = use syslog in foreground\n" );
	printf(" -o    = set log filename\n" );
	printf(" -l    = set log level 0-7\n" );
	printf(" -m    = set log mask (0xfffffff0)\n" );
	printf(" -n    = set application name\n" );
	printf(" -k    = set kiss mode on\n" );
	exit(0);
}

void wait_ms(int milliseconds) {
//    struct timespec req;
//    req.tv_sec = milliseconds / 1000;
//    req.tv_nsec = (milliseconds % 1000) * 1000000;
//    nanosleep(&req, NULL);
    return;
}

int linediscp ( int chan ) {
	struct termios t;
#define MAX_LINE_LENGTH 256
#define LDISC_MASK 	31
	char line[MAX_LINE_LENGTH];
	char *discpname;
	int discpnum = 0;
	int discpfound = 0;
	char *token;

	if (tcgetattr(chan, &t) != 0) PANIC;

	// Get the line discipline ID (an integer)
	int ldisc = t.c_lflag & LDISC_MASK;

	FILE *file = fopen("/proc/tty/ldiscs", "r");
    if (file == NULL) {
        perror("Error opening /proc/tty/ldiscs file");
        return (1);
    }
    while (fgets(line, MAX_LINE_LENGTH-1, file) != NULL) {
    	discpname = token = strtok(line, " ");
		token = strtok(NULL, " ");
		discpnum = atoi(token);
//    	if (sscanf(line, "%s %i", discpname, &discpnum) == 1) { // Parse using sscanf
    		if ( discpnum == ldisc ) {
        	    printf("Line discipline is %s\n", discpname);
        	    discpfound = 1;
        	    break;
    		}
//    	} else {
//    	    printf("Error parsing line: %s\n", line); // Handle parsing errors
//   	    return (2);
//    	}
    }

    fclose(file);
    if (discpfound) return(0);
    printf("no match found for line discipline in /proc/tty/ldiscs\nI will try to guess:\n");

	// You can then match the ID with the defined line discipline constants
#define LDISC_TTY      0
#define LDISC_SLIP     1
#define LDISC_PPP      3
#define LDISC_IRDA     11
#define LDISC_AX25     5
#define LDISC_STRIP    4
#define LDISC_MOUSE    2
#define LDISC_X25      6
#define LDISC_6PACK    7
#define LDISC_R3964    9
#define LDISC_HDLC     13
#define LDISC_SYNC_PPP 14
#define LDISC_HCI      15
#define LDISC_GIGASET_M101    16
#define LDISC_PPS      18

	switch (ldisc) {
	    case LDISC_TTY:
	        printf("Line discipline might be: TTY\n (The default line discipline, providing transparent operation (raw mode) as well as the habitual terminal line editing capabilities (cooked mode). )\n");
	        break;
	    case LDISC_SLIP:
	        printf("Line discipline might be: SLIP\n (Serial Line IP (SLIP) protocol processor for transmitting TCP/IP packets over serial lines.)\n");
	        break;
	    case LDISC_PPP:
	        printf("Line discipline might be: PPP\n (Point to Point Protocol (PPP) processor for transmitting network packets over serial lines.)\n");
	        break;
	    case LDISC_IRDA:
	        printf("Line discipline might be: IRDA\n (Linux IrDa (infrared data transmission) driver - see http://irda.sourceforge.net/ )\n");
	        break;
	    case LDISC_AX25:
	        printf("Line discipline might be: AX25\n (mkiss.)\n");
	        break;
	    case LDISC_STRIP:
	        printf("Line discipline might be: STRIP\n (Starmode Radio IP is a protocol designed (specifically for a range of Metricom radio modems)\n  for a research project conducted by Stanford University called the MosquitoNet Project.)\n");
	        break;
	    case LDISC_MOUSE:
	        printf("Line discipline might be: MOUSE\n (Device driver for RS232 connected pointing devices (serial mice).)\n");
	        break;
	    case LDISC_X25:
	        printf("Line discipline might be: X25\n (Line driver for transmitting X.25 packets over asynchronous serial lines.)\n");
	        break;
	    case LDISC_6PACK:
	        printf("Line discipline might be: 6PACK\n (A transmission protocol for data exchange between the PC and the TNC over a serial line.\n  It pre-dates AX-25 and can be used as an alternative to KISS.)\n");
	        break;
	    case LDISC_R3964:
	        printf("Line discipline might be: R3964\n (Line driver for Simatic R3964 module..)\n");
	        break;
	    case LDISC_HDLC:
	        printf("Line discipline might be: HDLC\n (Synchronous HDLC driver.)\n");
	        break;
	    case LDISC_SYNC_PPP:
	        printf("Line discipline might be: Synchronous PPP\n (Synchronous PPP driver.)\n");
	        break;
	    case LDISC_HCI:
	        printf("Line discipline might be: HCI\n (Bluetooth HCI UART driver.)\n");
	        break;
	    case LDISC_GIGASET_M101:
	        printf("Line discipline might be: Gigaset M101\n (Driver for Siemens Gigaset M101 serial DECT adapter.)\n");
	        break;
	    case LDISC_PPS:
	        printf("Line discipline might be: PPS\n (Driver for serial line Pulse Per Second (PPS) source.)\n");
	        break;
	    default:
	        printf("Line discipline : I have no idea!\n");
	        break;
	}
	return (0);
}

/*
 * Setup the communications port
 */
void comm_init(const char * portname)
{
struct termios t;

     chan = open(portname, O_RDWR|O_NOCTTY);
     if (chan == -1) {
    	 perror("Error opening serial port");
    	 PANIC;
     }
     if ( mydaemon.log_level > LOG_WARNING ) termios(chan);
     if (tcgetattr(chan, &t) != 0) PANIC;
     t.c_cc[VMIN] = 1;            /* Wake up after 1
                                    * characters arrive.
                                    */
     t.c_cc[VTIME] = 5;            /* Wake up 0.5 seconds
                                    * after the first char
                                    * arrives.
                                    */
                                   /* The combination of
                                    * VMIN/VTIME will cause
                                    * the program to wake up
                                    * 0.5 seconds after the
                                    * first character arrives
                                    * or after 1 characters
                                    * arrive whichever comes
                                    * first.
                                    */
     t.c_iflag &= ~(BRKINT         /* Ignore break       */
      | IGNPAR | PARMRK |          /* Ignore parity      */
          INPCK |                  /* Ignore parity      */
          ISTRIP |                 /* Don't mask         */
     INLCR | IGNCR | ICRNL         /* No <cr> or <lf>    */
      | IXON);                     /* Ignore STOP char   */
//     t.c_iflag |= IGNBRK | IXOFF;  /* Ignore BREAK
    t.c_iflag |= IGNBRK ;          /* Ignore BREAK        */
     t.c_oflag &= ~(OPOST);        /* No output flags     */
     t.c_lflag &= ~(               /* No local flags.  In */
          ECHO|ECHOE|ECHOK|ECHONL| /* particular, no echo */
          ICANON |                 /* no canonical input  */
                                   /* processing,         */
          ISIG |                   /* no signals,         */
          NOFLSH |                 /* no queue flush,     */
          TOSTOP);                 /* and no job control.
                                    */
     t.c_cflag &= (                /* Clear out old bits  */
          CSIZE |                  /* Character size      */
          CSTOPB |                 /* Two stop bits       */
//          HUPCL |                  /* Hangup on last close*/
          PARENB);                 /* Parity              */
//     t.c_cflag |= CLOCAL | CREAD | CS8;
     t.c_cflag |= HUPCL | CLOCAL | CREAD | CS8;
                                   /* CLOCAL => No modem
                                    * CREAD  => Enable
                                    *           receiver
                                    * CS8    => 8-bit data
                                    */

/* Copy input and output speeds into
 * struct termios t
 */
     if (cfsetispeed(&t, B9600) == -1) PANIC;
     if (cfsetospeed(&t, B9600) == -1) PANIC;

/* Throw away any input data (noise) */
     if (tcflush(chan, TCIFLUSH) == -1) PANIC;

/* Now, set the termial port attributes */
     if (tcsetattr(chan,TCSANOW, &t) == -1) PANIC;

     return;
}

/*
 * Here is the receive process.  The call to
 * listen() never returns.
 */
void listenpk(void)
{
char buf[BUFFSIZE];
int  count;
char * outbuf = NULL;
char * noerrors = NULL;
     while(1)                       /* Loop forever */
	 {
		  count = read(chan, &buf, BUFFSIZE);
		  if (count < 0) PANIC;
//          for (int i=0;i<count;i++) {
//        	  if ( buf[i] == '\n') buf[i] = '\000';
//          }
//          if ( interpretkiss ) {

		  if ( printnum &&  mydaemon.log_level > LOG_WARNING ) printf("interpretkiss = %i\n",interpretkiss);
		  printnum=0;
		  if ( interpretkiss ) {
			  if ( count ) mkiss_receive_buf(NULL, buf, noerrors, count);
			  if (ax->bump ) {
				  outbuf = (char *)ax->rbuff;
				  count = ax->count;
				  if (  mydaemon.log_level > LOG_WARNING ) printf("count,buf=%i,%s\n",count,(char *)ax->rbuff);
				  trace (outbuf, count);
				  ax->count = ax->bump = 0;
				  if ( mydaemon.log_level < LOG_WARNING ) count = 0;
			  } else {
				  count = 0;
			  }
		  } else {
			  outbuf = buf;
		  }
		  if (count) (void)write(STDOUT_FILENO,outbuf,count);
	 }
}


int main( int argc, char **argv ) {
	int   ca;
	pid_t pid = 0;
    const char *portname = "/dev/ttyMP6"; // Replace with your serial port
//    struct termios tty;
    struct termios t;
    char ch;
//    struct tm tm;
//    char message[180];
//    char buffer[80];
#define FEND   0xC0 // Frame  End
#define FESC   0xdb // Frame  Escape
#define TFEND  0xdc // Transposed Frame End
#define TFESC  0xdd // Transposed Frame Escape
#define HOCTL  0xff // Host off control command
#define H0POL  0x0e // Host poll of modem 0 command

	uid_t uid;
    char crlf[3] = {'\r', '\n', '\0'};
    char ctlcx3[4] = {'\003', '\003', '\003', '\000'};
    char kisson[] =  {'k', 'i', 's', 's', ' ', 'o', 'n', '\r', '\n', '\000'};
    char kissoff[] = {'k', 'i', 's', 's', ' ', 'o', 'f', 'f', '\000'};
    char kisskissoff[3] = {FEND, HOCTL, FEND};
    char kisstoss[3] = {FEND, '\000', FEND};
    char kisspoll[3] = {FEND, H0POL, FEND};

	mydaemon.appname = argv[0];
#ifdef DEBUG
	printf("DEBUG defined: default to foreground\n" );
	mydaemon.daemonize = 0;
#else	// DEBUG
	mydaemon.daemonize = 1;
#endif	// DEBUG
	mydaemon.log_level = LOG_WARNING;
//	mydaemon.sig_handler = c2m_signal;
	mydaemon.log_mask = 0xfffffff0;
	mydaemon.sig_handler = NULL;
	mydaemon.log_name = argv[0];
	mydaemon.use_syslog = 1;

	ax->bump = 0; // initialize the print buffer ready flag
	ax->count = 0;	// initialize the print buffer character count to zero

// Get the real user ID
	uid = getuid();
	struct passwd *pw = getpwuid(uid);
	openlog (NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    if (pw) {
	    syslog (LOG_NOTICE, "Program started by User %s on device %s", pw->pw_name, portname);
	} else {
		syslog (LOG_NOTICE, "Program started by User %d on device %s", uid, portname);
	}

    /* read command line options */
    	while ((ca = getopt(argc, argv, "hvfsko:l:m:n:")) != -1)
    	{
    		switch (ca)
    		{
    		case 'v':
    			printf("%s Version: %d.%d\n", argv[0], VER_MAJOR, VER_MINOR);
    			exit(0);
    			break;
    		case 'f':
    			mydaemon.daemonize = 0;
    			printf("run in foreground\n" );
    			break;
    		case 's':
    			mydaemon.use_syslog = 1;
    			printf("use syslog in foreground\n" );
    			break;

    		case 'o':
    			mydaemon.log_filename = optarg;
    			printf("set log filename to %s\n", optarg );
    			break;

    		case 'l':
    			mydaemon.log_level = atoi(optarg);
    			printf("set log level to %i\n" , atoi(optarg) );
    			break;

    		case 'm':
    			if ( (strlen(optarg) > 2) && (optarg[1] == 'x') )
    				sscanf(optarg,"%x",&mydaemon.log_mask);
    			else
    				mydaemon.log_mask = atoi(optarg);
    			printf("set log mask to 0x%06x\n", atoi(optarg) );
    			break;

    		case 'n':
    			mydaemon.appname = optarg;
    			mydaemon.log_name = optarg;
    			printf("set application name to %s\n", optarg );
    			break;

    		case 'k':
    			setkiss = 1;
    			fprintf(stderr,"Using KISS mode\n");
    			break;

    		case 'h':
    		default:
    			testpk232_usage();
    			break;
    		}
    	}

    af_daemon_set( &mydaemon );

    // Initialize the serial port
    if ( mydaemon.log_level > LOG_WARNING ) printf("open the serial port\n");
    comm_init(portname);

    if ( mydaemon.log_level > LOG_WARNING ) termios(chan);
    if ( mydaemon.log_level > LOG_WARNING ) printf( "serial line status:\n" );
    if ( mydaemon.log_level > LOG_WARNING ) checkserial(chan);
//    void cfmakeraw(struct termios *termios_p);
/*
    if (tcgetattr(STDIN_FILENO,&t) != 0) PANIC;
    t.c_lflag &= ~(ICANON | ECHO);  // Turn off the flags for echo and canonical input processing.
    if (tcflush(STDIN_FILENO, TCIFLUSH) == -1) PANIC; // Throw away any data (noise) on STDIN
    if (tcsetattr(STDIN_FILENO,TCSANOW, &t) == -1) PANIC; // Now, set the STDIN attributes
*/
    if (tcgetattr(chan,&t) != 0) PANIC; // re load t with the data for the serial port

    if (ax_open()) PANIC;
    if ( setkiss ) interpretkiss = 1;

    if ( ( pid = fork() ) == 0) listenpk(); // Call listenpk() as a new process.

// send the KISS stop command
    if ( mydaemon.log_level > LOG_WARNING ) printf("output kisskissoff\n");
	if (write(chan,kisskissoff,3) != 3) PANIC;
//	wait_ms(1000); // wait a sec
	sleep(1);
// toss 3 <CRTL>C's (this should force a cmd: prompt)
	if ( mydaemon.log_level > LOG_WARNING ) printf("\noutput 3x <ctrl>C\'s\n");
	if (write(chan,ctlcx3,strlen(ctlcx3)) != strlen(ctlcx3)) PANIC;
//	wait_ms(1000); // wait a sec
	sleep(1);
    // toss a cr/lf
	if ( mydaemon.log_level > LOG_WARNING ) printf("\ntoss a crlf\n");
	if (write(chan,crlf,strlen(crlf)) != strlen(crlf)) PANIC;
//	wait_ms(1000); // wait a sec
	sleep(1);
	if ( setkiss ) {
		// turn on kiss mode
		if ( mydaemon.log_level > LOG_WARNING ) printf("\nturn on kiss mode\n");
		if (write(chan,kisson,strlen(kisson)) != strlen(kisson)) PANIC;
		sleep (1);
		unsigned int xxx2 = TIOCM_RTS;
		if ( ioctl(chan, TIOCMBIS, &xxx2 ) == -1) PANIC;
		sleep (1);
	} else {
		// send the KISS stop command
		if ( mydaemon.log_level > LOG_WARNING ) printf("\noutput kisskissoff\n");
		if (write(chan,kisskissoff,3) != 3) PANIC;
		sleep(1);
		// toss 3 <CRTL>C's (this should force a cmd: prompt)
		if ( mydaemon.log_level > LOG_WARNING ) printf("\noutput 3x <ctrl>C\'s\n");
		if (write(chan,ctlcx3,strlen(ctlcx3)) != strlen(ctlcx3)) PANIC;
		sleep(1);
		// turn off kiss mode
		if (write(chan,kissoff,strlen(kissoff)) != strlen(kissoff)) PANIC;
	}
	// toss another cr/lf
	if ( !setkiss ) {
		if ( mydaemon.log_level > LOG_WARNING ) printf("\ntoss another crlf\n");
		if (write(chan,crlf,strlen(crlf)) != strlen(crlf)) PANIC;
	}
//	wait_ms(1000); // wait a sec
	sleep(1);
// let's double check the line discipline...
	if ( mydaemon.log_level > LOG_WARNING ) linediscp ( chan );
// and serial status lines
	if ( mydaemon.log_level > LOG_WARNING ) printf( "serial line status:\n" );
	if ( mydaemon.log_level > LOG_WARNING ) checkserial(chan);

    while (1)           /* Loop forever */
		 {
             // Copy standard input to the comm port.
			 (void)read(STDIN_FILENO,&ch,1);
			 // detect new line and change to <CR><LF>
			 if (ch == '\n') {
				 if (write(chan,crlf,2) != 2) PANIC;
			 } else {
				if (ch == '\x18') {
				  printf("\nControl \'X\' detected: exiting\n");
				  // send the KISS stop command
				  if ( setkiss ) {
					  if ( mydaemon.log_level > LOG_WARNING ) printf("output kisskissoff\n");
					  if (write(chan,kisskissoff,3) != 3) PANIC;
					  sleep (1);
				  }
				  break;
				} else if (ch == '\x19') {
					  if ( setkiss ) {
						  printf("\nControl \'Y\' detected: tossing an empty command to the PackRat\n");
						  if (write(chan,kisstoss,3) != 3) PANIC;
						  sleep (1);
					  }
				} else if (ch == '\x10') {
					  if ( setkiss ) {
						  printf("\nControl \'P\' detected: tossing a poll command to the PackRat\n");
						  if (write(chan,kisspoll,3) != 3) PANIC;
						  sleep (1);
					  }
				} else if (ch == '\x14') {
					  if ( setkiss ) {
						  printf("\nControl \'T\' detected: setting RTS high\n");
						  // Set RTS high (request to send)
						  unsigned int xxx = TIOCM_RTS;
						  if ( ioctl(chan, TIOCMBIS, &xxx ) == -1) PANIC;
						  sleep (1);
					  }
				} else {
					if (write(chan,&ch,1) != 1) PANIC;
				}
			 }
         }

    if ( pid ) {
    	if ( mydaemon.log_level > LOG_WARNING ) printf("terminating child...\n");
		if (kill(pid, SIGTERM) == -1) {
			perror("kill");
			exit(EXIT_FAILURE);
		}
		wait(NULL); // Wait for the child to terminate
		printf("Child process terminated\n");
    }
    if ( mydaemon.log_level > LOG_WARNING ) printf( "final serial line status:\n" );
    if ( mydaemon.log_level > LOG_WARNING ) checkserial(chan);
    close(chan);
	puts("pk232 disconnected!!!");
	return EXIT_SUCCESS;
}

int trace (char * buff, int size) {
//  Dump the kiss packet to the terminal in a format similar to the AEA PK232 TRACE command
//
//	W2JUP*>TESTER <UI>:
//	This is a test message packet.
//
//	Byte                 Hex                   Shifted ASCII      ASCII
//	000: A88AA6A8 8AA460AE 6494AAA0 406103F0   TESTER0W2JUP 0.x   ......`.d...@a..
//	010: 54686973 20697320 61207465 7374206D   *449.49.0.:29:.6   This is a test m
//	020: 65737361 67652070 61636B65 742E0D     299032.80152:..    essage packet...
//

#define MIN(x,y) ( ( x < y ) ? (x) : (y) )
	time_t currentTime;
	struct tm *localTime;
	char formattedTime[80];
	unsigned char ctemp;

	if ( mydaemon.log_level > LOG_WARNING ) {
		ctemp = *(buff+80);
		*(buff+80)='\000';
		if ( mydaemon.log_level > LOG_WARNING ) printf("size,buff=%i,%s\n",size,buff);
		*(buff+80)=ctemp;
	}
	// Get current time
	currentTime = time(NULL);
	// Convert to local time
	localTime = localtime(&currentTime);
	// Format the time
		if (localTime == NULL) {
		perror("localtime");
		return 1;
	}
	formattedTime[0] = '\0';  // Initialize the string
	if (strftime(formattedTime, 80, "%Y-%m-%d %H:%M:%S", localTime) == 0) {
		perror("strftime");
		return 2;
	}
	// Print the formatted time
	printf("Received: %s\n", formattedTime);
	printf("Byte                 Hex                  Shifted ASCII     ASCII\n");
	int j=0;
	int remaining = size;
	int count;
	char hex[47];
	char * hexp;
	unsigned char shiftasc[27];
	unsigned char asc[27];
	unsigned char uc;
	unsigned char uas;
	int i;
	while (j<size) {
		count = MIN(remaining,16);
		hexp = &hex[0];
		for (i=0;i<37;i++) hex[i] = ' ';
		for (i=0;i<17;i++) shiftasc[i] = asc[i] = ' ';
		for (i=0;i<count;i++) {
			sprintf(hexp,"%02x",*(unsigned char *)(buff+size-remaining+i));
			hexp+=2;
			if ( (i+1)%4 == 0 ) {
				*hexp = ' ';
				hexp++;
			}
			uas = *(buff+size-remaining+i);
			uc = uas >> 1;
			if ( !isprint(uc) ) uc = ' ';
			shiftasc[i] = uc;
			if ( !isprint(uas) ) uas = ' ';
			asc[i] = uas;
		}
		*hexp = ' ';
		hex[36] = shiftasc[16] = asc[16] = '\000';
		printf("%03x: %s %s  %s\n", size-remaining, hex, shiftasc, asc);
//		printf("%03x: %s count = %i\n", size-remaining, hex, count);
		j+=count;
		remaining-=count;
	}
	printf("\n");
	// try to decode the packect
	unpacket (buff, size);
	return (0);
}

void strip_trailing_blanks(char *str) {
  if (str == NULL) {
    return;
  }

  int index = strlen(str) - 1;
  while (index >= 0 && isspace(str[index])) {
    index--;
  }

  str[index + 1] = '\0';
}

int unpacket (char * buff, int size) {
// unpack the HLDC info and try to duplicate the normal PK232 output
// (assumes modulo 8 operation format for AX25 control frames)

		int numcalls = 0;
		char calllist[100][7];
		unsigned char uids[100];
		unsigned char commandbits[100];

		int i = 7;
		int j;
		int hasPID;
		char * frametype;
		char * supcontrol;
		char * unumcontrol;
		unsigned int pollbit;
		unsigned int sendseq;
		unsigned int recseq;
		char * tempbuff;


#define IFRAME	0	// 	Information frame
#define SFRAME	1	// 	Supervisory frame
#define UFRAME	3	// 	Unnumbered frame
#define RR		0	//  Receive Ready (RR)
#define RNR		1	//  Receive Not Ready (RNR)
#define REJ		2	//  Reject (REJ)
#define SREJ	3	//  Selective Reject (SREJ
#define SABME 0x0fu	//  Set Asynchronous Balanced Mode Extended (SABME)
#define SABM 0x07u	//  Set Asynchronous Balanced Mode (SABM)
#define DISC 0x08u	//  Disconnect (DISC)
#define DM   0x03u	//  Disconnect Mode (DM)
#define UA   0x0cu	//  Unnumbered Acknowledge (UA)
#define FRMR 0x11u	//  Frame Reject (FRMR)
#define UI   0x00u	//  Unnumbered Information (UI)
#define XID  0x17u	//  Exchange Identification (XID)
#define TEST 0x1cu	//  Test (TEST)

	if (size < 14) return (1);
	// (first byte is always zero, ie, buff[0]==0)
	if ( *buff ) {
		printf("Therte is garbage in the first byte of the message. We will not attempt decoding...\n");
		return(2);
	}
	// find last call
	while(i<(size-1)){
		// moves shifted ascii into calllist array
//		printf("i,buff[1],buff[i]= %i, %02x, %02x\n",i,(unsigned char)*(buff+1),(unsigned char)*(buff+i));
//		printf("i-7+1,buff[i-7+1],&shifted..=%i,%02x,%02x\n",i-7+1,(unsigned char)(*(buff+i-7+1)),(unsigned char)(*(buff+i-7+1))>>1);
		for (j=0;j<6;j++) calllist[numcalls][j] = (unsigned char)(*(buff+i-7+j+1)) >> 1;
		calllist[numcalls][6] = '\000'; // set the last byte to zero so we can use it as a string
		strip_trailing_blanks(&(calllist[numcalls][0]));
		uids[numcalls] = (*(buff+i-1) >> 1) & '\x0f';
		if ( (*(buff+i-1) >> 7) & 1 ) {
			commandbits[numcalls] = '*';
		} else {
			commandbits[numcalls] = ' ';
		}
		numcalls++;
//		printf("*(buff+i)=,%02x\n",*(buff+i));
		if ( *(buff+i) & 1 ) break;
		i+=7;
	}
	printf("call to %s-%u%c from %s-%u%c", (char*)calllist[0], uids[0], commandbits[0], (char*)calllist[1], uids[1], commandbits[1]);
	if ( numcalls > 2) {
		printf(" relay via ");
		for (j=2;j<numcalls;j++) printf(" %s-%u%c",(char*)calllist[j], uids[j], commandbits[j]);
	}
	printf("\n");
	// decode the control byte
	pollbit = ( ( (*(buff+i)) >> 4 ) & 1);
	switch (*(buff+i) & 3) {
	    case IFRAME:
	    	frametype = "Information frame";
			sendseq = ( ( (*(buff+i)) >> 1 ) & 7);
			recseq  = ( ( (*(buff+i)) >> 5 ) & 7);
			printf("%s with poll bit = %u, xmt seq. no. = %u,rec seq. no. = %u ", frametype, pollbit, sendseq, recseq);
			i+=checkPID(*(buff+i+1));
			printf("\n");
	    	break;
	    case SFRAME:
	    	frametype = "Supervisory frame";
	    	recseq  = ( ( (*(buff+i)) >> 5 ) & 7);
	    	switch ( ( (*(buff+i)) >> 2 ) & 3) {
	    	case RR:
	    		supcontrol = "Receive Ready";
	    		break;
	    	case RNR:
	    		supcontrol = "Receive Not Ready";
	    		break;
	    	case REJ:
	    		supcontrol = "Reject";
	    		break;
	    	case SREJ:
	    		supcontrol = "Selective Reject";
	    		break;
	    	default:
	    		PANIC;
	    	}
	    	printf("%s (%s) with poll bit = %u, rec seq. no. = %u\n", frametype, supcontrol, pollbit, recseq);
	    	break;
	    case UFRAME:
	    	frametype = "Unnumbered frame";
	    	hasPID = 0;
	    	switch ( ( ( (*(buff+i)) >> 2 ) & 3) & ( ( (*(buff+i)) >> 3 ) & 0x1cu )) {
			case SABME:
				unumcontrol = "Set Asynchronous Balanced Mode Extended";
				break;
			case SABM:
				unumcontrol = "Set Asynchronous Balanced Mode";
				break;
			case DISC:
				unumcontrol = "Disconnect";
				break;
			case DM:
				unumcontrol = "Disconnect Mode";
				break;
			case UA:
				unumcontrol = "Unnumbered Acknowledge";
				break;
			case FRMR:
				unumcontrol = "Frame Reject";
				break;
			case UI:
				unumcontrol = "Unnumbered Information";
				hasPID = 1;
				break;
			case XID:
				unumcontrol = "Exchange Identification";
				break;
			case TEST:
				unumcontrol = "Test";
				break;
			default:
				PANIC;
			}
	    	printf("%s (%s) with poll bit = %u " , frametype, unumcontrol, pollbit);
	    	if ( hasPID ) {
	    		i+=checkPID(*(buff+i+1));
	    	}
	    	printf("\n");
	    	break;
	    default:
//	    	PANIC;
	}
//	printf("size,i= %i, %i\n",size,i);
	if ( (i+2) > size ) return (0);  // no info data
	tempbuff = (char*)malloc((size_t)size+10);
	if (tempbuff == NULL) PANIC;
	memcpy(tempbuff, buff, size);
//	printf("memcpy done\n");
	for (j=i;j<size;j++) {
		if ( !isprint( *(tempbuff+j) ) ) tempbuff[j] = ' ';
	}
	*(tempbuff+size) = '\000';
	printf("content: [%s]\n",tempbuff+i+3);
	printf("############################################################################\n");
	free(tempbuff);

	return (0);
}

int checkPID(unsigned char PID){
	if ( PID == 0xff ) return (1);
	if ( ( (PID & 0x30) == 0x20 ) || ( (PID & 0x30) == 0x10 ) ) printf ("AX.25 Layer 3 Implemented. ");
	switch ( PID & 0xcf ) {
		    case 0x01:
				printf("ISO 8208/CCITT X.25 PLP ");
		    	break;
		    case 0x06:
				printf("Compressed TCP/IP Packet Van Jacobson (RFC 1144) ");
		    	break;
		    case 0x07:
				printf("Uncompressed TCP/IP Packet Van Jacobson (RFC 1144) ");
		    	break;
		    case 0x08:
				printf("Segmentation Fragment ");
		    	break;
		    case 0xC3:
				printf("TEXNET Datagram Protocol ");
		    	break;
		    case 0xC4:
				printf("Link Quality Protocol ");
		    	break;
		    case 0xCA:
				printf("Appletalk ");
		    	break;
		    case 0xCB:
				printf("Appletalk ARP ");
		    	break;
		    case 0xCC:
				printf("ARPA Internet Protocol ");
		    	break;
		    case 0xCD:
				printf("ARPA Address Resolution Protocol ");
		    	break;
		    case 0xCE:
				printf("FlexNet ");
		    	break;
		    case 0xCF:
				printf("NET/ROM ");
		    	break;
		    case 0xF0:
				printf("No Layer 3 Protocol ");
		    	break;
		    default:
//		    	PANIC;
		    	return(0);
		    	break;
	}
	printf("\n");
	return (0);
}
