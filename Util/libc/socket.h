#ifndef	SOCKET_H_
#define	SOCKET_H_

#include"basicdef.h"

#if defined(WIN32) || defined(_WIN64)
	#include<ws2tcpip.h>
	#include<iphlpapi.h>
	#include<winnetwk.h>
	#include<mswsock.h>
	#include<mstcpip.h>
	typedef SOCKET				FD_SOCKET;
	typedef	WSABUF				SOCKBUF;
	#define	GetSocketError()	WSAGetLastError()
	#define	SetSocketError(e)	WSASetLastError(e)
	enum SOCKET_ERROR_CODE{
		ERROR_EAFNOSUPPORT = WSAEAFNOSUPPORT,
		ERROR_EINPROGRESS = WSAEINPROGRESS,
		ERROR_ETIMEDOUT = WSAETIMEDOUT,
		ERROR_ECONNABORTED = WSAECONNABORTED,
	};
	#pragma comment(lib,"wsock32.lib")
	#pragma comment(lib,"ws2_32.lib")
	#pragma comment(lib,"iphlpapi.lib")
#else
	#include<sys/socket.h>
	#if defined(__FreeBSD__) || defined(__APPLE__)
		#include<net/if_dl.h>
	#elif __linux__
		#include<netpacket/packet.h>
	#endif
	#include<netdb.h>
	#include<poll.h>
	#include<arpa/inet.h>
	#include<netinet/in.h>
	#include<net/if.h>
	#include<net/if_arp.h>
	#include<netinet/ip.h>
	#include<netinet/ip6.h>
	#include<netinet/if_ether.h>
	#include<netinet/ip_icmp.h>
	#include<netinet/icmp6.h>
	#include<netinet/igmp.h>
	#include<netinet/tcp.h>
	#include<netinet/udp.h>
	#include<ifaddrs.h>
	#include<sys/uio.h>
	#ifndef	SOCKET_ERROR
		#define	SOCKET_ERROR	(-1)
	#endif
	#ifndef	INVALID_SOCKET
		#define	INVALID_SOCKET	(-1)
	#endif
	typedef	int					FD_SOCKET;
	typedef	struct iovec		SOCKBUF;
	#define	GetSocketError()	(errno)
	#define	SetSocketError(e)	((errno) = (e))
	enum SOCKET_ERROR_CODE{
		ERROR_EAFNOSUPPORT = EAFNOSUPPORT,
		ERROR_EINPROGRESS = EINPROGRESS,
		ERROR_ETIMEDOUT = ETIMEDOUT,
		ERROR_ECONNABORTED = ECONNABORTED,
	};
#endif

#ifdef	__cplusplus
extern "C"{
#endif

/* NETCARD */
EXEC_RETURN sock_SetupEnv(void);
EXEC_RETURN sock_CleanEnv(void);
unsigned int sock_GetInterfaceIndex(const char* ifname);
char* sock_GetInterfaceName(unsigned int ifindex,char* ifname_buf);
/* SOCKET */
struct addrinfo* sock_AddrInfoList(const char* host,const char* serv,int ai_socktype);
socklen_t sock_AddrSize(const struct sockaddr* addr);
EXEC_RETURN sock_Text2Addr(struct sockaddr* addr,int af,const char* strIP,unsigned short port);
EXEC_RETURN sock_Addr2Text(struct sockaddr* addr,char* strIP,unsigned short* port);
EXEC_RETURN sock_BindLocalAddr(FD_SOCKET sockfd,const struct sockaddr* addr);
FD_SOCKET sock_TcpConnect(const struct sockaddr* addr,int msec);
EXEC_RETURN sock_TcpListen(FD_SOCKET sockfd);
FD_SOCKET sock_TcpAccept(FD_SOCKET listenfd,struct sockaddr* from);
EXEC_RETURN sock_Close(FD_SOCKET sockfd);
EXEC_RETURN sock_Shut(FD_SOCKET sockfd);
EXEC_RETURN sock_ShutRD(FD_SOCKET sockfd);
EXEC_RETURN sock_ShutWR(FD_SOCKET sockfd);
/* SOCKET IO */
int sock_Send(FD_SOCKET sockfd,const void* buf,unsigned int nbytes,const struct sockaddr* to);
int sock_Recv(FD_SOCKET sockfd,void* buf,unsigned int nbytes,struct sockaddr* from);
void sock_BufStructInit(SOCKBUF* sbuf,const void* buf,unsigned int nbytes);
int sock_SendVec(FD_SOCKET sockfd,SOCKBUF* sbuf_vec,unsigned int vec_count);
int sock_RecvVec(FD_SOCKET sockfd,SOCKBUF* sbuf_vec,unsigned int vec_count);
int sock_TcpSendAll(FD_SOCKET sockfd,const void* buf,unsigned int nbytes);
int sock_TcpRecvAll(FD_SOCKET sockfd,void* buf,unsigned int nbytes);
void sock_PollStructSet(struct pollfd* poll_fd,FD_SOCKET fd);
int sock_Poll(struct pollfd* fdarray,unsigned long nfds,int msec);
int sock_Select(int nfds,fd_set* rset,fd_set* wset,fd_set* xset,int msec);
/* SOCKET OPTION */
int sock_Family(FD_SOCKET sockfd);
int sock_Type(FD_SOCKET sockfd);
EXEC_RETURN sock_Error(FD_SOCKET sockfd,int* error);
EXEC_RETURN sock_NonBlock(FD_SOCKET sockfd,int bool_val);
EXEC_RETURN sock_GetLocalAddr(FD_SOCKET sockfd,struct sockaddr* saddr);
EXEC_RETURN sock_GetPeerAddr(FD_SOCKET sockfd,struct sockaddr* saddr);
EXEC_RETURN sock_SetSendTimeOut(FD_SOCKET sockfd,int msec);
EXEC_RETURN sock_SetRecvTimeOut(FD_SOCKET sockfd,int msec);
int sock_GetSendBufSize(FD_SOCKET sockfd);
int sock_SetSendBufSize(FD_SOCKET sockfd,int size);
int sock_GetRecvBufSize(FD_SOCKET sockfd);
EXEC_RETURN sock_SetRecvBufSize(FD_SOCKET sockfd,int size);
EXEC_RETURN sock_SetUniCastTTL(FD_SOCKET sockfd,unsigned char ttl);
EXEC_RETURN sock_SetMultiCastTTL(FD_SOCKET sockfd,int ttl);
EXEC_RETURN sock_TcpGetMSS(FD_SOCKET sockfd,int* mss);
EXEC_RETURN sock_TcpSetMSS(FD_SOCKET sockfd,int mss);
EXEC_RETURN sock_TcpKeepPeerHostAlive(FD_SOCKET sockfd);
EXEC_RETURN sock_TcpEnableOOBInLine(FD_SOCKET sockfd,int bool_val);
int sock_TcpAtMark(FD_SOCKET sockfd);
EXEC_RETURN sock_TcpEnableLinger(FD_SOCKET sockfd,int bool_val,unsigned int sec);
EXEC_RETURN sock_UdpEnableBroadcast(FD_SOCKET sockfd,int bool_val);
EXEC_RETURN sock_UdpDisconnect(FD_SOCKET sockfd);
EXEC_RETURN sock_UdpMcastGroupJoin(FD_SOCKET sockfd,const struct sockaddr* grp,int bool_val);
EXEC_RETURN sock_UdpMcastEnableLoop(FD_SOCKET sockfd,int bool_val);
EXEC_RETURN sock_UdpMcastGetInterface(FD_SOCKET sockfd,struct in_addr* iaddr,unsigned int* ifindex);
EXEC_RETURN sock_UdpMcastSetInterface(FD_SOCKET sockfd,struct in_addr iaddr,unsigned int ifindex);
/* HTTP */
EXEC_RETURN sock_HttpOpenRequest(FD_SOCKET sockfd,const char* method,const char* url_path,const char* host);
EXEC_RETURN sock_HttpSetRequestHeader(FD_SOCKET sockfd,const char* header);
EXEC_RETURN sock_HttpSendRequest(FD_SOCKET sockfd,const void* option_data,size_t nbytes);
int sock_HttpRecvResponse(FD_SOCKET sockfd,char* buffer,size_t bufsize);
size_t sock_HttpGetContentLength(const char* response);
EXEC_RETURN sock_HttpRecvChunkSize(FD_SOCKET sockfd,size_t* chunksize);
char* sock_HttpBuildDataHeader(char* header,const char* boundary,const char* post_name,const char* local_path,const char* mime_type);
char* sock_HttpBuildDataEOF(char* eof,const char* boundary);

#ifdef	__cplusplus
}
#endif

#endif
