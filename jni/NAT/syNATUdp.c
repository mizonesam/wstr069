#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <linux/if.h>
#include <assert.h>

#include "syNATUdp.h"
#include "syCwmpCommon.h"

int openPort(unsigned short port, unsigned int interfaceIp) {
	int fd;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == INVALID_SOCKET) {
		int err = getErrno();
		EPrint("Could not create a UDP socket: %d\n", err);
		return INVALID_SOCKET;
	}

	struct sockaddr_in addr;
	memset((char*) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if ((interfaceIp != 0) && (interfaceIp != 0x100007f)) {
		addr.sin_addr.s_addr = htonl(interfaceIp);
 
		VPrint("Binding to interface: %x\n", htonl(interfaceIp));
 
	}

	if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
		int e = getErrno();

		switch (e) {
		case 0: {
			EPrint("Could not bind socket\n");
			return INVALID_SOCKET;
		}
		case EADDRINUSE: {
			EPrint("Port %d for receiving UDP is in use\n", port);
			return INVALID_SOCKET;
		}
			break;
		case EADDRNOTAVAIL: {

			EPrint("Cannot assign requested address\n");
 
			return INVALID_SOCKET;
		}
			break;
		default: {
			EPrint("Could not bind UDP receive port,Error=%d %s\n", e, strerror(e));
			return INVALID_SOCKET;
		}
			break;
		}
	}
 
	VPrint("Opened port %d with fd %d\n", port, fd);
 
	assert(fd != INVALID_SOCKET);

	return fd;
}

int getMessage(int fd, char* buf, int* len, unsigned int* srcIp,
		unsigned short* srcPort) {
	assert(fd != INVALID_SOCKET);

	int originalSize = *len;
	assert(originalSize > 0);

	struct sockaddr_in from;
	int fromLen = sizeof(from);

	*len = recvfrom(fd, buf, originalSize, 0, (struct sockaddr *) &from,
			(socklen_t*) &fromLen);

	if (*len == SOCKET_ERROR) {
		int err = getErrno();

		switch (err) {
		case ENOTSOCK:
			EPrint("Error fd not a socket\n");
			break;
		case ECONNRESET:
			EPrint("Error connection reset - host not reachable\n");
			break;

		default:
			EPrint("Socket Error=%d\n", err);
			break;
		}

		return SY_FAILED;
	}

	if (*len < 0) {
		VPrint("socket closed? negative len\n");
		return SY_FAILED;
	}

	if (*len == 0) {
		VPrint("socket closed? zero len\n");
		return SY_FAILED;
	}

	*srcPort = ntohs(from.sin_port);
	*srcIp = ntohl(from.sin_addr.s_addr);

	if ((*len) + 1 >= originalSize) {
 
		VPrint("Received a message that was too large\n");
 
		return SY_FAILED;
	}
	buf[*len] = 0;

	return SY_SUCCESS;
}

int sendMessage(int fd, char* buf, int l, unsigned int dstIp,
		unsigned short dstPort) {
	assert(fd != INVALID_SOCKET);

	int s;
	if (dstPort == 0) {
		// sending on a connected port
		assert(dstIp == 0);

		s = send(fd, buf, l, 0);
	} else {
		assert(dstIp != 0);
		assert(dstPort != 0);

		struct sockaddr_in to;
		int toLen = sizeof(to);
		memset(&to, 0, toLen);

		to.sin_family = AF_INET;
		to.sin_port = htons(dstPort);
		to.sin_addr.s_addr = htonl(dstIp);

		s = sendto(fd, buf, l, 0, (struct sockaddr*) &to, toLen);
	}

	if (s == SOCKET_ERROR) {
		int e = getErrno();
		switch (e) {
		case ECONNREFUSED:
		case EHOSTDOWN:
		case EHOSTUNREACH: {
			// quietly ignore this
		}
			break;
		case EAFNOSUPPORT: {
			EPrint("err EAFNOSUPPORT in send\n");
		}
			break;
		default: {
			EPrint("err %d - (%s) in send\n", e, strerror(e));
		}
			break;
		}
		return SY_FAILED;
	}

	if (s == 0) {
		EPrint("no data sent in send\n");
		return SY_FAILED;
	}

	if (s != l) { 
		EPrint("only %d out of %d bytes sent\n", s, l); 
		return SY_FAILED;
	}

	return SY_SUCCESS;
}

int getLocalIp(unsigned int* ip)    
{          
	int MAXINTERFACES=16;    
	int fd, intrface, retn = 0;      
	struct ifreq buf[MAXINTERFACES];      
	struct ifconf ifc;      

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)      
	{      
		ifc.ifc_len = sizeof(buf);      
		ifc.ifc_buf = (caddr_t)buf;      
		if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))      
		{      
			intrface = ifc.ifc_len / sizeof(struct ifreq);      

			while (intrface-- > 0)      
			{      
				if (!(ioctl(fd, SIOCGIFADDR, (char *) &buf[intrface])))      
				{      
					*ip = ntohl(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr.s_addr);
					close(fd);      
					return SY_SUCCESS;
				}                          
			}    
		}      
		close(fd);      
	}    
	return SY_FAILED;	
}

