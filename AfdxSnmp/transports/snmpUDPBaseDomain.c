/* UDP base transport support functions
 */

#include <net-snmp-config.h>

#include <types.h>
#include <library/snmpUDPBaseDomain.h>

#include <afdx/afdx_GenericTX.h>

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <mswsock.h>
#include <errno.h>

#include <types.h>
#include <library/snmpSocketBaseDomain.h>
#include <library/snmpUDPDomain.h>
#include <library/snmp_debug.h>
#include <library/tools.h>
#include <library/default_store.h>
#include <library/system.h>
#include <library/snmp_assert.h>

#define MSG_DONTWAIT 0
#define MSG_NOSIGNAL 0

void
_netsnmp_udp_sockopt_set(int fd, int local)
{
	/*
	 * SO_REUSEADDR will allow multiple apps to open the same port at
	 * the same time. Only the last one to open the socket will get
	 * data. Obviously, for an agent, this is a bad thing. There should
	 * only be one listener.
	 */

	/*
	 * Try to set the send and receive buffers to a reasonably large value, so
	 * that we can send and receive big PDUs (defaults to 8192 bytes (!) on
	 * Solaris, for instance).  Don't worry too much about errors -- just
	 * plough on regardless.
	 */
	netsnmp_sock_buffer_set(fd, SO_SNDBUF, local, 0);
	netsnmp_sock_buffer_set(fd, SO_RCVBUF, local, 0);
}

#define IP_SENDSRCADDR IP_RECVDSTADDR /* DragonFly BSD */
#define netsnmp_udpbase_recvfrom_sendto_defined

enum {
	cmsg_data_size = sizeof(struct in_pktinfo)
};

static LPFN_WSARECVMSG pfWSARecvMsg;
static LPFN_WSASENDMSG pfWSASendMsg;


int
netsnmp_udpbase_recvfrom(int s, void *buf, int len, struct sockaddr *from,
socklen_t *fromlen, struct sockaddr *dstip,
socklen_t *dstlen, int *if_index)
{
	int r;
	WSABUF wsabuf;
	char cmsg[WSA_CMSG_SPACE(sizeof(struct in_pktinfo))];
	WSACMSGHDR *cm;
	WSAMSG msg;
	DWORD bytes_received;

	wsabuf.buf = buf;
	wsabuf.len = len;

	msg.name = from;
	msg.namelen = *fromlen;
	msg.lpBuffers = &wsabuf;
	msg.dwBufferCount = 1;
	msg.Control.len = sizeof(cmsg);
	msg.Control.buf = cmsg;
	msg.dwFlags = 0;

	if (pfWSARecvMsg) {
		r = pfWSARecvMsg(s, &msg, &bytes_received, NULL, NULL) == 0 ?
		bytes_received : -1;
		*fromlen = msg.namelen;
	}
	else {
		r = recvfrom(s, buf, len, MSG_DONTWAIT, from, fromlen);
	}

	if (r == -1) {
		return -1;
	}

	DEBUGMSGTL(("udpbase:recv", "got source addr: %s\n",
		inet_ntoa(((struct sockaddr_in *)from)->sin_addr)));

	{
		/* Get the local port number for use in diagnostic messages */
		int r2 = getsockname(s, dstip, dstlen);
		netsnmp_assert(r2 == 0);
	}

	for (cm = WSA_CMSG_FIRSTHDR(&msg); cm; cm = WSA_CMSG_NXTHDR(&msg, cm)) {
		if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_PKTINFO) {
			struct in_pktinfo* src = (struct in_pktinfo *)WSA_CMSG_DATA(cm);
			netsnmp_assert(dstip->sa_family == AF_INET);
			((struct sockaddr_in*)dstip)->sin_addr = src->ipi_addr;
			*if_index = src->ipi_ifindex;
			DEBUGMSGTL(("udpbase:recv",
				"got destination (local) addr %s, iface %d\n",
				inet_ntoa(src->ipi_addr), *if_index));
		}
	}
	return r;
}

int netsnmp_udpbase_sendto(int fd, struct in_addr *srcip, int if_index,
struct sockaddr *remote, void *data, int len, netsnmp_address_list * address)
{

	WSABUF        wsabuf;
	WSAMSG        m;
	char          cmsg[WSA_CMSG_SPACE(sizeof(struct in_pktinfo))];
	DWORD         bytes_sent;
	int           rc;

	wsabuf.buf = data;
	wsabuf.len = len;

	memset(&m, 0, sizeof(m));
	m.name = remote;
	m.namelen = sizeof(struct sockaddr_in);
	m.lpBuffers = &wsabuf;
	m.dwBufferCount = 1;

	if (pfWSASendMsg && srcip && srcip->s_addr != INADDR_ANY) {
		WSACMSGHDR *cm;

		DEBUGMSGTL(("udpbase:sendto", "sending from [%d] %s\n", if_index,
			inet_ntoa(*srcip)));

		memset(cmsg, 0, sizeof(cmsg));

		m.Control.buf = cmsg;
		m.Control.len = sizeof(cmsg);

		cm = WSA_CMSG_FIRSTHDR(&m);
		cm->cmsg_len = WSA_CMSG_LEN(cmsg_data_size);
		cm->cmsg_level = IPPROTO_IP;
		cm->cmsg_type = IP_PKTINFO;

		{
			struct in_pktinfo ipi = { 0 };
			ipi.ipi_ifindex = if_index;
			ipi.ipi_addr.s_addr = srcip->s_addr;
			memcpy(WSA_CMSG_DATA(cm), &ipi, sizeof(ipi));
		}

		rc = pfWSASendMsg(fd, &m, 0, &bytes_sent, NULL, NULL);
		if (rc == 0)
			return bytes_sent;
		DEBUGMSGTL(("udpbase:sendto", "sending from [%d] %s failed: %d\n",
			if_index, inet_ntoa(*srcip), WSAGetLastError()));
	}
	//rc = sendto(fd, data, len, 0, remote, sizeof(struct sockaddr));

	/*#################################################*/
	/*#################################################*/
	/*#################################################*/
	/*#################################################*/
	//for (int i = 0, *p = data; i < len; i++){
	//printf("0x%08x", p++);
	//}
	
	const u_char *p;
	p = data;
	int snmp_data[2048];
	for (int i = 0; i < len; i++, p++){
		//printf("%d ", *p);
		snmp_data[i] = *p;
	}
	rc = l_TransmitAFDXSetup(1, 0, address, snmp_data, len);
	
	/*#################################################*/
	/*#################################################*/
	/*#################################################*/
	/*#################################################*/

	return rc;
}

/*
 * You can write something into opaque that will subsequently get passed back
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...
 */

int
netsnmp_udpbase_recv(netsnmp_transport *t, void *buf, int size,
void **opaque, int *olength)
{
	int             rc = -1;
	socklen_t       fromlen = sizeof(netsnmp_sockaddr_storage);
	netsnmp_indexed_addr_pair *addr_pair = NULL;
	struct sockaddr *from;

	if (t != NULL && t->sock >= 0) {
		addr_pair = (netsnmp_indexed_addr_pair *)malloc(sizeof(netsnmp_indexed_addr_pair));
		if (addr_pair == NULL) {
			*opaque = NULL;
			*olength = 0;
			return -1;
		}
		else {
			memset(addr_pair, 0, sizeof(netsnmp_indexed_addr_pair));
			from = &addr_pair->remote_addr.sa;
		}

		while (rc < 0) {
			socklen_t local_addr_len = sizeof(addr_pair->local_addr);
			rc = netsnmp_udp_recvfrom(t->sock, buf, size, from, &fromlen,
				(struct sockaddr*)&(addr_pair->local_addr),
				&local_addr_len, &(addr_pair->if_index));
			if (rc < 0 && errno != EINTR) {
				break;
			}
		}

		if (rc >= 0) {
			DEBUGIF("netsnmp_udp") {
				char *str = netsnmp_udp_fmtaddr(
					NULL, addr_pair, sizeof(netsnmp_indexed_addr_pair));
				DEBUGMSGTL(("netsnmp_udp",
					"recvfrom fd %d got %d bytes (from %s)\n",
					t->sock, rc, str));
				free(str);
			}
		}
		else {
			DEBUGMSGTL(("netsnmp_udp", "recvfrom fd %d err %d (\"%s\")\n",
				t->sock, errno, strerror(errno)));
		}
		*opaque = (void *)addr_pair;
		*olength = sizeof(netsnmp_indexed_addr_pair);
	}
	return rc;
}



int
netsnmp_udpbase_send(netsnmp_transport *t, void *buf, int size,
void **opaque, int *olength, netsnmp_address_list * address)
{
	int rc = -1;
	netsnmp_indexed_addr_pair *addr_pair = NULL;
	struct sockaddr *to = NULL;

	if (opaque != NULL && *opaque != NULL &&
		((*olength == sizeof(netsnmp_indexed_addr_pair) ||
		(*olength == sizeof(struct sockaddr_in))))) {
		addr_pair = (netsnmp_indexed_addr_pair *)(*opaque);
	}
	else if (t != NULL && t->data != NULL &&
		t->data_length == sizeof(netsnmp_indexed_addr_pair)) {
		addr_pair = (netsnmp_indexed_addr_pair *)(t->data);
	}

	to = &addr_pair->remote_addr.sa;

	if (to != NULL && t != NULL && t->sock >= 0) {
		DEBUGIF("netsnmp_udp") {
			char *str = netsnmp_udp_fmtaddr(NULL, (void *)addr_pair,
				sizeof(netsnmp_indexed_addr_pair));
			DEBUGMSGTL(("netsnmp_udp", "send %d bytes from %p to %s on fd %d\n",
				size, buf, str, t->sock));
			free(str);
		}
		while (rc < 0) {
			rc = netsnmp_udp_sendto(t->sock,
				addr_pair ? &(addr_pair->local_addr.sin.sin_addr) : NULL,
				addr_pair ? addr_pair->if_index : 0, to, buf, size, address);
			if (rc < 0 && errno != EINTR) {
				DEBUGMSGTL(("netsnmp_udp", "sendto error, rc %d (errno %d)\n",
					rc, errno));
				break;
			}
		}
	}
	return rc;
}

void
netsnmp_udp_base_ctor(void)
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	GUID WSARecvMsgGuid = WSAID_WSARECVMSG;
	GUID WSASendMsgGuid = WSAID_WSASENDMSG;
	DWORD nbytes;
	int result;

	netsnmp_assert(s != SOCKET_ERROR);
	/* WSARecvMsg(): Windows XP / Windows Server 2003 and later */
	result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&WSARecvMsgGuid, sizeof(WSARecvMsgGuid),
		&pfWSARecvMsg, sizeof(pfWSARecvMsg), &nbytes, NULL, NULL);
	if (result == SOCKET_ERROR)
		DEBUGMSGTL(("netsnmp_udp", "WSARecvMsg() not found (errno %ld)\n",
		WSAGetLastError()));

	/* WSASendMsg(): Windows Vista / Windows Server 2008 and later */
	result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&WSASendMsgGuid, sizeof(WSASendMsgGuid),
		&pfWSASendMsg, sizeof(pfWSASendMsg), &nbytes, NULL, NULL);
	if (result == SOCKET_ERROR)
		DEBUGMSGTL(("netsnmp_udp", "WSASendMsg() not found (errno %ld)\n",
		WSAGetLastError()));

	closesocket(s);
}