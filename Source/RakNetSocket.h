/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* RakNet 套接字封装类 */

#pragma once
#include "RakNetTypes.h"
#include "RakNetDefines.h"
#include "Export.h"
#include "SocketIncludes.h"
#include "RakAssert.h"
#include "SocketDefines.h"
#include "MTUSize.h"

namespace RakNet
{

struct RAK_DLL_EXPORT RakNetSocket
{
public:
	RakNetSocket();
	~RakNetSocket() noexcept;

/* 	void Accept( */
/* 		struct sockaddr *addr, */
/* 		int *addrlen); */

	inline int Connect(
		const struct sockaddr *name,
		int namelen) {return connect__(s,name,namelen);}

	static RakNetSocket* Create
#ifdef __native_client__
		(_PP_Instance_ _chromeInstance);
#else
		(int af,
		int type,
		int protocol);
#endif

	int Bind(		
		const struct sockaddr *addr,
		int namelen);

	inline int GetSockName(		
		struct sockaddr *name,
			socklen_t * namelen) {return getsockname__(s,name,namelen);}

	inline int GetSockOpt (
		int level,
		int optname,
		char * optval,
		socklen_t *optlen) {return getsockopt__(s,level,optname,optval,optlen);}


	int IOCTLSocket(		
		long cmd,
		unsigned long *argp);

	int Listen (		
		int backlog);

	inline int Recv(		
		char * buf,
		int len,
		int flags) {return recv__(s,buf,len,flags);}

	inline int RecvFrom(		
		char * buf,
		int len,
		int flags,
		struct sockaddr * from,
			socklen_t * fromlen) {return recvfrom__(s,buf,len,flags,from,fromlen);}

/* 	inline int Select( */
/* 		int nfds, */
/* 		fd_set *readfds, */
/* 		fd_set *writefds, */
/* 		fd_set *exceptfds, */
/* 		struct timeval *timeout) {return select__(nfds,readfds,writefds,exceptfds,timeout);} */

	inline int Send(		
		const char * buf,
		int len,
		int flags) {return send__(s,buf,len,flags);}

	inline int SendTo(		
		const char * buf,
		int len,
		int flags,
		const struct sockaddr *to,
		int tolen) {return sendto__(s,buf,len,flags,to,tolen);}

	#ifdef _WIN32
	#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	#else
	inline int Fcntl(int cmd, int arg) {return fcntl(s,cmd,arg);}
	#endif


#if defined(_WIN32) && !defined(WINDOWS_STORE_RT)
	inline int _WSASendTo(
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
	{ return WSASendTo(s,lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,lpTo,iTolen,lpOverlapped,lpCompletionRoutine);}

#endif
	
	int SetSockOpt(		
		int level,
		int optname,
		const char * optval,
		int optlen);

	int Shutdown(		
		int how);


	inline void SetRemotePortRakNetWasStartedOn(unsigned short i) {remotePortRakNetWasStartedOn_PS3_PSP2=i;}
	inline void SetUserConnectionSocketIndex(unsigned int i) {userConnectionSocketIndex=i;}
	inline void SetBoundAddress(SystemAddress i) {boundAddress=i;}
	inline void SetSocketFamily(unsigned short i) {socketFamily=i;}
	inline void SetBlockingSocket(bool i) {blockingSocket=i;}
	inline void SetExtraSocketOptions(unsigned int i) {extraSocketOptions=i;}
	inline void SetChromeInstance(_PP_Instance_ i) {chromeInstance=i;}
	inline void SetBoundAddressToLoopback(unsigned char ipVersion) {boundAddress.SetToLoopback(ipVersion);}

	inline SystemAddress GetBoundAddress(void) const {return boundAddress;}
	inline unsigned short GetRemotePortRakNetWasStartedOn(void) const {return remotePortRakNetWasStartedOn_PS3_PSP2;}
	inline bool GetBlockingSocket() {return blockingSocket;}
	inline unsigned int GetExtraSocketOptions(void) const {return extraSocketOptions;}
	inline unsigned short GetSocketFamily(void) const {return socketFamily;}
	inline _PP_Instance_ GetChromeInstance(void) const {return chromeInstance;}
	inline unsigned int GetUserConnectionSocketIndex(void) const {
		RakAssert(userConnectionSocketIndex!=(unsigned int)-1);
		return userConnectionSocketIndex;}


#ifdef __native_client__
		/* 标志：指示当前是否有 SendTo 操作正在进行 */
		bool sendInProgress;

		/* 下一个排队等待发送的数据包数据（当 nextSendSize > 0 时有效） */
		char nextSendBuffer[MAXIMUM_MTU_SIZE];

		/* 下一个排队等待发送的数据包大小，为 0 表示没有排队的数据包 */
		int nextSendSize;

		/* 排队数据包的目标地址 */
		PP_NetAddress_Private nextSendAddr;
#endif

		__UDPSOCKET__ s;

protected:

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	void* recvEvent;
#endif

	#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	/*
	 * PS3: 使用信令时为 PS3 设置。
	 * PS3: 使用信令返回的端口进行连接。将此值设置为 RakNet 实际启动时使用的端口。
	 * PSP2: 设置为非零值以使用 SCE_NET_SOCK_DGRAM_P2P。此选项适用于 ad-hoc 模式或与之配合使用。
	 */
	#endif

	unsigned short remotePortRakNetWasStartedOn_PS3_PSP2;
	unsigned int userConnectionSocketIndex;
	SystemAddress boundAddress;
	unsigned short socketFamily;
	bool blockingSocket;
	unsigned int extraSocketOptions;
	_PP_Instance_ chromeInstance;
};

} /* RakNet 命名空间 */
