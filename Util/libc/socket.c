#include"socket.h"
#ifdef	__cplusplus
extern "C"{
#endif

#if !defined(WIN32) && !defined(_WIN64)
static void sig_free_zombie(int sig){
	int status;
	while(waitpid(-1,&status,WNOHANG) > 0);
}
#endif

static unsigned short check_sum(unsigned short* buffer,int iSize){
	unsigned long ulCksum = 0;
	while(iSize > 1){
		ulCksum += *buffer++;
		iSize -= sizeof(unsigned short);
	}
	if(iSize)
		ulCksum += *(unsigned char*)buffer;
	ulCksum = (ulCksum >> 16) + (ulCksum & 0xffff);
	ulCksum += (ulCksum >> 16);
	return ~ulCksum;
}

/* NETCARD */
EXEC_RETURN sock_SetupEnv(void){
#if defined(WIN32) || defined(_WIN64)
	WSADATA data;
	return WSAStartup(MAKEWORD(2,2),&data) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct sigaction act,oact;
	if(signal(SIGPIPE,SIG_IGN) == SIG_ERR)
		return EXEC_ERROR;
	act.sa_handler = sig_free_zombie;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	return sigaction(SIGCHLD,&act,&oact) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sock_CleanEnv(void){
#if defined(WIN32) || defined(_WIN64)
	int res = WSACleanup();
	return (res != 0 && WSAGetLastError() != WSANOTINITIALISED) ? EXEC_ERROR:EXEC_SUCCESS;
#else
	Sigfunc_t old0,old1;
	old0 = signal(SIGPIPE,SIG_DFL);
	old1 = signal(SIGCHLD,SIG_DFL);
	return (old0==SIG_ERR || old1==SIG_ERR) ? EXEC_ERROR:EXEC_SUCCESS;
#endif
}

unsigned int sock_GetInterfaceIndex(const char* ifname){
	/* ifindex != 0 */
	return if_nametoindex(ifname);
}

char* sock_GetInterfaceName(unsigned int ifindex,char* ifname_buf){
	/* buflen <= IF_NAMESIZE */
	return if_indextoname(ifindex,ifname_buf);
}

/* SOCKET */
struct addrinfo* sock_AddrInfoList(const char* host,const char* serv,int ai_socktype){
	struct addrinfo* res = NULL;
	struct addrinfo hinfo = {0};/* some fields must clear zero */
	hinfo.ai_flags = AI_CANONNAME;
	hinfo.ai_socktype = ai_socktype;
	getaddrinfo(host,serv,&hinfo,&res);
	return res;
}

socklen_t sock_AddrSize(const struct sockaddr* addr){
	socklen_t slen = 0;
	if(addr->sa_family == AF_INET)
		slen = sizeof(struct sockaddr_in);
	else if(addr->sa_family == AF_INET6)
		slen = sizeof(struct sockaddr_in6);
	else
	SetSocketError(ERROR_EAFNOSUPPORT);
	return slen;
}

EXEC_RETURN sock_Text2Addr(struct sockaddr* addr,int af,const char* strIP,unsigned short port){
	if(af == AF_INET){/* IPv4 */
		struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
		addr_in->sin_family = AF_INET;
		addr_in->sin_port = htons(port);
		if(!strIP || !strIP[0])
			addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
		else{
			unsigned int net_addr = inet_addr(strIP);
			if(net_addr == INADDR_NONE)
				return EXEC_ERROR;
			else
				addr_in->sin_addr.s_addr = net_addr;
		}
		return EXEC_SUCCESS;
	}
	else if(af == AF_INET6){
		struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)addr;
		addr_in6->sin6_family = AF_INET6;
		addr_in6->sin6_port = htons(port);
		if(strIP && strIP[0]){
#if defined(WIN32) || defined(_WIN64)
			int len = sizeof(struct sockaddr_in6);
			return WSAStringToAddressA((char*)strIP,AF_INET6,NULL,(struct sockaddr*)addr_in6,&len) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
			return inet_pton(AF_INET6,strIP,addr_in6->sin6_addr.s6_addr) == 1 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
		}
		else
			addr_in6->sin6_addr = in6addr_any;
		return EXEC_SUCCESS;
	}
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_Addr2Text(struct sockaddr* addr,char* strIP,unsigned short* port){
	int res;
	unsigned long len;
	if(addr->sa_family == AF_INET){
		struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;
		len = INET_ADDRSTRLEN + 6;
		if(port)
			*port = ntohs(addr_in->sin_port);
#if defined(WIN32) || defined(_WIN64)
		res = WSAAddressToStringA(addr,sizeof(struct sockaddr_in),NULL,strIP,&len);
		if(!res && strstr(strIP,":"))
			*strstr(strIP,":") = 0;
		return res == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
		return inet_ntop(AF_INET,&addr_in->sin_addr,strIP,len) ? EXEC_SUCCESS:EXEC_ERROR;
#endif
	}
	else if(addr->sa_family == AF_INET6){
		char buf[INET6_ADDRSTRLEN + 8];
		struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)addr;
		len = sizeof(buf);
		if(port)
			*port = ntohs(addr_in6->sin6_port);
#if defined(WIN32) || defined(_WIN64)
		res = WSAAddressToStringA(addr,sizeof(struct sockaddr_in6),NULL,buf,&len);
		if(!res){
			if(*buf == '['){
				*strstr(buf,"]") = 0;
				strcpy(strIP,buf + 1);
			}else
				strcpy(strIP,buf);
		}
		return res == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
		return inet_ntop(AF_INET6,&addr_in6->sin6_addr,strIP,len) ? EXEC_SUCCESS:EXEC_ERROR;
#endif
	}
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_BindLocalAddr(FD_SOCKET sockfd,const struct sockaddr* addr){
	int on = 1;
	socklen_t slen = sock_AddrSize(addr);
	if(!slen)
		return EXEC_ERROR;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)(&on),sizeof(on))<0)
		return EXEC_ERROR;
	return bind(sockfd,addr,slen) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

FD_SOCKET sock_TcpConnect(const struct sockaddr* addr,int msec){
	FD_SOCKET sockfd = INVALID_SOCKET;
	int res,error;
	/* get struct length */
	socklen_t slen = sock_AddrSize(addr);
	if(!slen)
		goto end;
	/* create a TCP socket */
	sockfd = socket(addr->sa_family,SOCK_STREAM,0);
	if(sockfd == INVALID_SOCKET)
		goto end;
	/* if set timedout,must set sockfd nonblock */
	if(msec >= 0)
		sock_NonBlock(sockfd,1);
	/* try connect */
	res = connect(sockfd,addr,slen);
	if(!res){
		/* connect success,destination maybe localhost */
	}
	/* occur other error...connect failure */
	else if(GetSocketError() != ERROR_EINPROGRESS){
		error = GetSocketError();
		sock_Close(sockfd);
		SetSocketError(error);
		sockfd = INVALID_SOCKET;
		goto end;
	}else{
	/* wait connect finish...
		if timedout or socket occur error...we must close socket to stop 3 times handshank. 
	*/
		struct timeval tval;
		fd_set rset,wset;
		FD_ZERO(&rset);
		FD_SET(sockfd,&rset);
		wset = rset;
		tval.tv_sec = msec / 1000;
		tval.tv_usec = msec % 1000 * 1000;
		res = select(sockfd + 1,&rset,&wset,NULL,&tval);/* wait tval */
		if(res == 0){/* timeout */
			sock_Close(sockfd);
			SetSocketError(ERROR_ETIMEDOUT);
			sockfd = INVALID_SOCKET;
			goto end;
		}else if(FD_ISSET(sockfd,&rset) || FD_ISSET(sockfd,&wset)){/* maybe success,maybe error */
			/* check error occur ??? */
			socklen_t len = sizeof(error);
			error = 0;/* need clear */
			res = getsockopt(sockfd,SOL_SOCKET,SO_ERROR,(char*)&error,&len);
			if(res < 0)/* solaris */
				error = errno;
			if(error){
				sock_Close(sockfd);
				SetSocketError(error);
				sockfd = INVALID_SOCKET;
				goto end;
			}
		}else{/* select error,must close socket */
			error = GetSocketError();
			sock_Close(sockfd);
			SetSocketError(error);
			sockfd = INVALID_SOCKET;
			goto end;
		}		
	}
	/* socket connect success,then reset block io */
	if(msec >= 0)
		sock_NonBlock(sockfd,0);
end:
	return sockfd;
}

EXEC_RETURN sock_TcpListen(FD_SOCKET sockfd){
	return listen(sockfd,SOMAXCONN) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

FD_SOCKET sock_TcpAccept(FD_SOCKET listenfd,struct sockaddr* from){
	FD_SOCKET confd;
	socklen_t slen = sizeof(struct sockaddr_storage);
	socklen_t* p_slen = from ? &slen:NULL;
	do{
		confd = accept(listenfd,from,p_slen);
	}while(confd == INVALID_SOCKET && (GetSocketError() == ERROR_ECONNABORTED || errno == EPROTO || errno == EINTR));
	return confd;
}

EXEC_RETURN sock_Close(FD_SOCKET sockfd){
	/* suggest to use shutdown */
#if defined(WIN32) || defined(_WIN64)
	return closesocket(sockfd) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return close(sockfd) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sock_Shut(FD_SOCKET sockfd){
#if defined(WIN32) || defined(_WIN64)
	return shutdown(sockfd,SD_BOTH) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return shutdown(sockfd,SHUT_RDWR) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sock_ShutRD(FD_SOCKET sockfd){
#if defined(WIN32) || defined(_WIN64)
	return shutdown(sockfd,SD_RECEIVE) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return shutdown(sockfd,SHUT_RD) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sock_ShutWR(FD_SOCKET sockfd){
#if defined(WIN32) || defined(_WIN64)
	return shutdown(sockfd,SD_SEND) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return shutdown(sockfd,SHUT_WR) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

/* SOCKET IO */
int sock_Send(FD_SOCKET sockfd,const void* buf,unsigned int nbytes,const struct sockaddr* to){
	socklen_t slen = 0;
	if(to){
		slen = sock_AddrSize(to);
		if(!slen)
			return -1;
	}
	return sendto(sockfd,(char*)buf,nbytes,0,to,slen);
}

int sock_Recv(FD_SOCKET sockfd,void* buf,unsigned int nbytes,struct sockaddr* from){
	socklen_t slen = sizeof(struct sockaddr_storage);
	socklen_t* p_slen = from ? &slen:NULL;
	return recvfrom(sockfd,(char*)buf,nbytes,0,from,p_slen);
}

void sock_BufStructInit(SOCKBUF* sbuf,const void* buf,unsigned int nbytes){
#if defined(WIN32) || defined(_WIN64)
	sbuf->buf = (char*)buf;
	sbuf->len = nbytes;
#else
	sbuf->iov_base = (void*)buf;
	sbuf->iov_len = nbytes;
#endif
}

int sock_SendVec(FD_SOCKET sockfd,SOCKBUF* sbuf_vec,unsigned int vec_count){
#if defined(WIN32) || defined(_WIN64)
	DWORD realbytes;
	if(WSASend(sockfd,sbuf_vec,vec_count,&realbytes,0,NULL,NULL))
		return -1;
	return realbytes;
#else
	return writev(sockfd,sbuf_vec,vec_count);
#endif
}

int sock_RecvVec(FD_SOCKET sockfd,SOCKBUF* sbuf_vec,unsigned int vec_count){
#if defined(WIN32) || defined(_WIN64)
	DWORD realbytes;
	if(WSARecv(sockfd,sbuf_vec,vec_count,&realbytes,0,NULL,NULL))
		return -1;
	return realbytes;
#else
	return readv(sockfd,sbuf_vec,vec_count);
#endif	
}

int sock_TcpSendAll(FD_SOCKET sockfd,const void* buf,unsigned int nbytes){
	int wn = 0;
	int res;
	while(wn < nbytes){
		res = send(sockfd,((char*)buf) + wn,nbytes - wn,0);
		if(res > 0)
			wn += res;
		else
			return wn ? wn:res;
	}
	return nbytes;
}

int sock_TcpRecvAll(FD_SOCKET sockfd,void* buf,unsigned int nbytes){
#ifdef	MSG_WAITALL
	return recv(sockfd,(char*)buf,nbytes,MSG_WAITALL);
#else
	int rn = 0;
	int res;
	while(rn < nbytes){
		res = recv(sockfd,((char*)buf) + rn,nbytes - rn,0);
		if(res > 0)
			rn += res;
		else
			return rn ? rn:res;
	}
	return nbytes;
#endif
}

void sock_PollStructSet(struct pollfd* poll_fd,FD_SOCKET fd){
	poll_fd->fd = fd;
	poll_fd->events = POLLIN|POLLOUT;
	poll_fd->revents = 0;
}

int sock_Poll(struct pollfd* fdarray,unsigned long nfds,int msec){
	int res;
	do{
#if defined(WIN32) || defined(_WIN64)
		res = WSAPoll(fdarray,nfds,msec);
#else
		res = poll(fdarray,nfds,msec);
#endif
	}while(res < 0 && errno == EINTR);
	return res;
}

int sock_Select(int nfds,fd_set* rset,fd_set* wset,fd_set* xset,int msec){
	int res;
	struct timeval tval;
	do{
		if(msec >= 0){
			tval.tv_sec = msec / 1000;
			tval.tv_usec = msec % 1000 * 1000;
		}
		res = select(nfds,rset,wset,xset,msec < 0 ? NULL:&tval);
	}while(res < 0 && errno == EINTR);
	return res;
}

/* SOCKET OPTION */
int sock_Family(FD_SOCKET sockfd){
	struct sockaddr_storage ss; 
	socklen_t len = sizeof(ss);
	if(getsockname(sockfd,(struct sockaddr*)&ss,&len))
		return -1;
	return ss.ss_family;
}

int sock_Type(FD_SOCKET sockfd){
	int type;
	socklen_t len = sizeof(type);
	if(getsockopt(sockfd,SOL_SOCKET,SO_TYPE,(char*)&type,&len) < 0)
		type = -1;
	return type;
}

EXEC_RETURN sock_Error(FD_SOCKET sockfd,int* error){
	socklen_t len = sizeof(int);
	return getsockopt(sockfd,SOL_SOCKET,SO_ERROR,(char*)error,&len) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_NonBlock(FD_SOCKET sockfd,int bool_val){
#if defined(WIN32) || defined(_WIN64)
	u_long ctl = bool_val ? 1:0;
	return ioctlsocket(sockfd,FIONBIO,&ctl) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	return ioctl(sockfd,FIONBIO,&bool_val) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sock_GetLocalAddr(FD_SOCKET sockfd,struct sockaddr* saddr){
	socklen_t slen = sizeof(struct sockaddr_storage);
	return getsockname(sockfd,saddr,&slen) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_GetPeerAddr(FD_SOCKET sockfd,struct sockaddr* saddr){
	socklen_t slen = sizeof(struct sockaddr_storage);
	return getpeername(sockfd,saddr,&slen) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_SetSendTimeOut(FD_SOCKET sockfd,int msec){
#if defined(WIN32) || defined(_WIN64)
	return setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char*)&msec,sizeof(msec)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct timeval tval;
	tval.tv_sec = msec / 1000;
	msec %= 1000;
	tval.tv_usec = msec * 1000;
	return setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char*)&tval,sizeof(tval)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

EXEC_RETURN sock_SetRecvTimeOut(FD_SOCKET sockfd,int msec){
#if defined(WIN32) || defined(_WIN64)
	return setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&msec,sizeof(msec)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#else
	struct timeval tval;
	tval.tv_sec = msec / 1000;
	msec %= 1000;
	tval.tv_usec = msec * 1000;
	return setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&tval,sizeof(tval)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
#endif
}

int sock_GetSendBufSize(FD_SOCKET sockfd){
	int res;
	socklen_t len = sizeof(res);
	if(getsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(char*)&res,&len))
		return -1;
	return res;
}

int sock_SetSendBufSize(FD_SOCKET sockfd,int size){
	return setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(char*)&size,sizeof(size));
}

int sock_GetRecvBufSize(FD_SOCKET sockfd){
	int res;
	socklen_t len = sizeof(res);
	if(getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(char*)&res,&len))
		return -1;
	return res;
}

EXEC_RETURN sock_SetRecvBufSize(FD_SOCKET sockfd,int size){
	return setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(char*)&size,sizeof(size)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_SetUniCastTTL(FD_SOCKET sockfd,unsigned char ttl){
	int val = ttl;
	int family = sock_Family(sockfd);
	if(family == AF_INET)
		return setsockopt(sockfd,IPPROTO_IP,IP_TTL,(char*)&val,sizeof(val)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	else if(family == AF_INET6)
		return setsockopt(sockfd,IPPROTO_IPV6,IPV6_UNICAST_HOPS,(char*)&val,sizeof(val)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_SetMultiCastTTL(FD_SOCKET sockfd,int ttl){
	int family = sock_Family(sockfd);
	if(family == AF_INET){
		unsigned char val = ttl > 0xff ? 0xff:ttl;
		return setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_TTL,(char*)&val,sizeof(val)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}else if(family == AF_INET6)
		return setsockopt(sockfd,IPPROTO_IPV6,IPV6_MULTICAST_HOPS,(char*)&ttl,sizeof(ttl)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_TcpGetMSS(FD_SOCKET sockfd,int* mss){
	socklen_t len = sizeof(*mss);
	return getsockopt(sockfd,IPPROTO_TCP,TCP_MAXSEG,(char*)mss,&len) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_TcpSetMSS(FD_SOCKET sockfd,int mss){
	return setsockopt(sockfd,IPPROTO_TCP,TCP_MAXSEG,(char*)&mss,sizeof(mss)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_TcpKeepPeerHostAlive(FD_SOCKET sockfd){
	int on = 1;
	return setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(char*)&on,sizeof(on)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_TcpEnableOOBInLine(FD_SOCKET sockfd,int bool_val){
	int on = (bool_val != 0);
	return setsockopt(sockfd,SOL_SOCKET,SO_OOBINLINE,(char*)&on,sizeof(on)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

int sock_TcpAtMark(FD_SOCKET sockfd){
#if defined(WIN32) || defined(_WIN64)
	u_long ok;
	return !ioctlsocket(sockfd,SIOCATMARK,&ok) ? ok:-1;
#else
	return sockatmark(sockfd);
#endif
}

EXEC_RETURN sock_TcpEnableLinger(FD_SOCKET sockfd,int bool_val,unsigned int sec){
	/* it is used by close()/closesocket() */
	struct linger sl = {0};
	sl.l_onoff = bool_val;
	sl.l_linger = sec;
	/* if(l_onoff == TRUE && sec == 0)
			close()/closesocket() will send RST to peer host;
	*/
	return setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(char*)&sl,sizeof(sl)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_UdpEnableBroadcast(FD_SOCKET sockfd,int bool_val){
	bool_val = bool_val ? 1:0;
	return setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char*)&bool_val,sizeof(bool_val)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_UdpDisconnect(FD_SOCKET sockfd){
	int res;
	struct sockaddr sa = {0};
	sa.sa_family = AF_UNSPEC;
	res = connect(sockfd,&sa,sizeof(sa));
	return (res != 0 && GetSocketError() != ERROR_EAFNOSUPPORT) ? EXEC_ERROR:EXEC_SUCCESS;
}

EXEC_RETURN sock_UdpMcastGroupJoin(FD_SOCKET sockfd,const struct sockaddr* grp,int bool_val){
	int optname = 0;
	if(grp->sa_family == AF_INET){/* IPv4 */
		struct ip_mreq req = {0};
		req.imr_interface.s_addr = htonl(INADDR_ANY);
		req.imr_multiaddr = ((struct sockaddr_in*)grp)->sin_addr;
		optname = bool_val ? IP_ADD_MEMBERSHIP:IP_DROP_MEMBERSHIP;
		return setsockopt(sockfd,IPPROTO_IP,optname,(char*)&req,sizeof(req)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}
	else if(grp->sa_family == AF_INET6){/* IPv6 */
		struct ipv6_mreq req = {0};
		req.ipv6mr_interface = 0;
		req.ipv6mr_multiaddr = ((struct sockaddr_in6*)grp)->sin6_addr;
		optname = bool_val ? IPV6_JOIN_GROUP:IPV6_LEAVE_GROUP;
		return setsockopt(sockfd,IPPROTO_IPV6,optname,(char*)&req,sizeof(req)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_UdpMcastEnableLoop(FD_SOCKET sockfd,int bool_val){
	int family = sock_Family(sockfd);
	if(family == AF_INET){
		unsigned char val = (bool_val != 0);
		return setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_LOOP,(char*)&val,sizeof(val)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}else if(family == AF_INET6){
		bool_val = (bool_val != 0);
		return setsockopt(sockfd,IPPROTO_IPV6,IPV6_MULTICAST_LOOP,(char*)&bool_val,sizeof(bool_val)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_UdpMcastGetInterface(FD_SOCKET sockfd,struct in_addr* iaddr,unsigned int* ifindex){
	int family = sock_Family(sockfd);
	socklen_t optlen;
	if(family == AF_INET){
		optlen = sizeof(struct in_addr);
		return getsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_IF,(char*)iaddr,&optlen) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}else if(family == AF_INET6){
		optlen = sizeof(unsigned int);
		return getsockopt(sockfd,IPPROTO_IPV6,IPV6_MULTICAST_IF,(char*)ifindex,&optlen) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	}
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

EXEC_RETURN sock_UdpMcastSetInterface(FD_SOCKET sockfd,struct in_addr iaddr,unsigned int ifindex){
	int family = sock_Family(sockfd);
	if(family == AF_INET)
		return setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_IF,(char*)&iaddr,sizeof(struct in_addr)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	else if(family == AF_INET6)
		return setsockopt(sockfd,IPPROTO_IPV6,IPV6_MULTICAST_IF,(char*)&ifindex,sizeof(ifindex)) == 0 ? EXEC_SUCCESS:EXEC_ERROR;
	SetSocketError(ERROR_EAFNOSUPPORT);
	return EXEC_ERROR;
}

/* HTTP */
EXEC_RETURN sock_HttpOpenRequest(FD_SOCKET sockfd,const char* method,const char* url_path,const char* host){
	char buffer[2048];
	if(strcmp(method,"GET") && strcmp(method,"POST"))
		return -1;
	/* send http basic request headers */
	sprintf(buffer,"%s %s HTTP/1.1\r\n\
Host: %s\r\n\
Accept: */*\r\n\
Connection: Keep-Alive\r\n",method,url_path,host);
	return sock_TcpSendAll(sockfd,buffer,strlen(buffer))==strlen(buffer) ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_HttpSetRequestHeader(FD_SOCKET sockfd,const char* header){
	/* send http request header */
	size_t header_len = header ? strlen(header):0;
	return sock_TcpSendAll(sockfd,header,header_len) == header_len ? EXEC_SUCCESS:EXEC_ERROR;
}

EXEC_RETURN sock_HttpSendRequest(FD_SOCKET sockfd,const void* option_data,size_t nbytes){
	/* send http request end flag */
	if(sock_TcpSendAll(sockfd,"\r\n",2) != 2)
		return EXEC_ERROR;
	/* http POST method may send more data after request */
	if(sock_TcpSendAll(sockfd,option_data,nbytes) != nbytes)
		return EXEC_ERROR;
	return EXEC_SUCCESS;
}

int sock_HttpRecvResponse(FD_SOCKET sockfd,char* buffer,size_t bufsize){
	int sts_code = -1;
	size_t rn = 0;
	memset(buffer,0,bufsize);
	while(rn + 1 < bufsize){
		if(recv(sockfd,buffer + rn,sizeof(char),0) <= 0)
			break;
		else if(rn >= 3 && 
				buffer[rn-3]=='\r' && buffer[rn-2]=='\n' && 
				buffer[rn-1]=='\r' && buffer[rn]=='\n'){
			char* p;
			buffer[rn + 1] = 0;
			/* get status code after get complete header */
			p = strstr(buffer," ");
			++p;
			sscanf(p,"%d",&sts_code);
			break;
		}
		++rn;
	}
	return sts_code;
}

size_t sock_HttpGetContentLength(const char* response){
	size_t Length = 0;
	char* p;
	p = strstr((char*)response,"Transfer-Encoding: chunked");
	if(p)
		return 0;
	p = strstr((char*)response,"Content-Length: ");
	if(p){
		p += strlen("Content-Length: ");
		sscanf(p,"%p",(void**)&Length);
	}
	return Length;
}

EXEC_RETURN sock_HttpRecvChunkSize(FD_SOCKET sockfd,size_t* chunksize){
	char buffer[12] = {0};
	size_t rn = 0;
	while(rn < sizeof(buffer)){
		if(recv(sockfd,buffer + rn,sizeof(char),0) <= 0)
			return EXEC_ERROR;
		else if(rn >= 2 && buffer[rn-1]=='\r' && buffer[rn]=='\n'){
			*chunksize = 0;
			sscanf(buffer,"%p",(void**)chunksize);
			return EXEC_SUCCESS;
		}
		++rn;
	}
	return EXEC_ERROR;
}

char* sock_HttpBuildDataHeader(char* header,const char* boundary,const char* post_name,const char* local_path,const char* mime_type){
	sprintf(header,"--%s\r\nContent-Disposition: form-data; name=\"%s\"",boundary,post_name);
	if(local_path && *local_path){
		sprintf(header,"%s; filename=\"%s\"",header,local_path);
		if(mime_type && *mime_type)
			sprintf(header,"%s\r\nContent-Type: %s",header,mime_type);
		else
			strcat(header,"\r\nContent-Type: text/plain");
	}
	strcat(header,"\r\n\r\n");
	return header;
}

char* sock_HttpBuildDataEOF(char* eof,const char* boundary){
	if(boundary && *boundary)
		sprintf(eof,"\r\n--%s--\r\n",boundary);
	else
		strcpy(eof,"\r\n");
	return eof;
}

#ifdef	__cplusplus
}
#endif
