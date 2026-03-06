/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*
 * 
 * SocketLayer 类实现
 *
 */




#pragma once
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "RakNetSmartPtr.h"
/* #include "RakNetSocket.h" */
#include "Export.h"
#include "MTUSize.h"
#include "RakString.h"

/* #include "ClientContextStruct.h" */

namespace RakNet
{
/* 前向声明 */
class RakPeer;

// class RAK_DLL_EXPORT SocketLayerOverride
// {
// public:
// 	SocketLayerOverride() {}
// 	virtual ~SocketLayerOverride() {}
//
// 	/* Called when SendTo would otherwise occur. */
// 	virtual int RakNetSendTo( const char *data, int length, const SystemAddress &systemAddress )=0;
//
// 	/* 当RecvFrom would otherwise occur. Return number of bytes read. Write data into dataOut时调用 */
// 	/* 返回 -1 使用 RakNet 的正常 recvfrom，0 中止 RakNet 的正常 recvfrom，正数表示返回数据 */
// 	virtual int RakNetRecvFrom( char dataOut[ MAXIMUM_MTU_SIZE ], SystemAddress *senderOut, bool calledFromMainThread )=0;
// };

/* A platform independent implementation of Berkeley sockets, with settings used by RakNet */
class RAK_DLL_EXPORT SocketLayer
{

public:
	
	/* 默认构造函数 */
	SocketLayer();
	
	/* 析构函数 */
	~SocketLayer() noexcept;
	
	/*
	 * Creates a bound socket to listen for incoming connections on the specified port
	 * 参数[输入] port the port number
	 * 参数[输入] blockingSocket
	 * 返回值: A new socket used for accepting clients
	 *
	static RakNetSocket* CreateBoundSocket( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, unsigned short socketFamily, _PP_Instance_ chromeInstance );
#if defined(WINDOWS_STORE_RT)
	static RakNetSocket* CreateWindowsStore8Socket( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, _PP_Instance_ chromeInstance );
#endif
	static RakNetSocket* CreateBoundSocket_IPV4( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, _PP_Instance_ chromeInstance );
	#if RAKNET_SUPPORT_IPV6==1
		static RakNetSocket* CreateBoundSocket_SupportIPV4And6( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, unsigned short socketFamily, _PP_Instance_ chromeInstance );
	#endif
	static RakNetSocket* CreateBoundSocket_PS3Lobby( unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned short socketFamily );
	static RakNetSocket* CreateBoundSocket_PSP2( unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned short socketFamily );
	*/

#ifndef WINDOWS_STORE_RT
	/*
	 * Returns if this specified port is in use, for UDP
	 * 参数[输入] port the port number
	 * 返回值: If this port is already in use
	 */
	/* static bool IsPortInUse_Old(unsigned short port, const char *hostAddress); */
	/* static bool IsPortInUse(unsigned short port, const char *hostAddress, unsigned short socketFamily ); */
	static bool IsSocketFamilySupported(const char *hostAddress, unsigned short socketFamily);
#endif

/* static constexpr char* DomainNameToIP_Old( const char *domainName ); */
/* static constexpr char* DomainNameToIP( const char *domainName ); */
	
	/*
	 * Write data of length length to writeSocket
	 * 参数[输入] writeSocket The socket to write to
	 * 参数[输入] data The data to write
	 * 参数[输入] length The length of data
	 */
	/* static void Write( RakNetSocket*writeSocket, const char* data, const int length ); */
	
	/*
	 * Read data from a socket
	 * 参数[输入] s the socket
	 * 参数[输入] rakPeer The instance of rakPeer containing the recvFrom C callback
	 * 参数[输入] errorCode An error code if an error occured .
	 * 参数[输入] connectionSocketIndex Which of the sockets in RakPeer we are using
	 * 返回值: Returns true if you successfully read data, false on error.
	 */
/* static void RecvFromBlocking_IPV4( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead ); */
/* 	#if RAKNET_SUPPORT_IPV6==1 */
/* 		static void RecvFromBlockingIPV4And6( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead ); */
/* 	#endif */
/* static void RecvFromBlocking( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead ); */
#if defined(WINDOWS_STORE_RT)
/* static void RecvFromBlocking_WindowsStore8( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead ); */
#endif

	/*
	 * Given a socket and IP, retrieves the subnet mask, on linux the socket is unused
	 * 参数[输入] inSock the socket
	 * 参数[输入] inIpString The ip of the interface you wish to retrieve the subnet mask from
	 * 返回值: Returns the ip dotted subnet mask 若成功, otherwise returns empty string ("")
	 */
	static RakNet::RakString GetSubNetForSocketAndIp(__UDPSOCKET__ inSock, RakNet::RakString inIpString);


	/*
	 * 将socket flags设置为nonblocking
	 * 参数[输入] listenSocket the socket to set
	 */
/* static void SetNonBlocking( RakNetSocket* listenSocket); */


	/*
	 * Retrieve all local IP address in a string format.
	 * 参数[输入] s The socket whose port we are referring to
	 * 参数[输入] ipList An array of ip address in dotted notation.
	 */
	static void GetMyIP( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] );

	
	/*
	 * 调用 sendto (UDP obviously)
	 * 参数[输入] s the socket
	 * 参数[输入] data The byte buffer to send
	 * 参数[输入] length The length of the data in bytes
	 * 参数[输入] ip The address of the remote host in dotted notation.
	 * 参数[输入] port The port number to send to.
	 * 返回值: 0 on success, nonzero on failure.
	 */
/* static int SendTo( UDPSOCKET s, const char *data, int length, const char ip[ 16 ], unsigned short port, unsigned short remotePortRakNetWasStartedOn_PS3, unsigned int extraSocketOptions, const char *file, const long line ); */

	/*
	 * 调用 sendto' (UDP obviously)
	 * It won't reach the recipient, except on a LAN
	 * However, this is good for opening routers / firewalls
	 * 参数[输入] s the socket
	 * 参数[输入] data The byte buffer to send
	 * 参数[输入] length The length of the data in bytes
	 * 参数[输入] ip The address of the remote host in dotted notation.
	 * 参数[输入] port The port number to send to.
	 * 参数[输入] ttl Max hops of datagram
	 * 返回值: 0 on success, nonzero on failure.
	 */
/* static int SendToTTL( RakNetSocket *s, const char *data, int length, SystemAddress &systemAddress, int ttl ); */

	/*
	 * 调用 sendto (UDP obviously)
	 * 参数[输入] s the socket
	 * 参数[输入] data The byte buffer to send
	 * 参数[输入] length The length of the data in bytes
	 * 参数[输入] binaryAddress The address of the remote host in binary format.
	 * 参数[输入] port The port number to send to.
	 * 返回值: 0 on success, nonzero on failure.
	 */
/* static int SendTo( RakNetSocket *s, const char *data, int length, SystemAddress systemAddress, const char *file, const long line ); */

/* static unsigned short GetLocalPort(RakNetSocket *s); */
	static unsigned short GetLocalPort( __UDPSOCKET__ s);
/* static void GetSystemAddress_Old ( RakNetSocket *s, SystemAddress *systemAddressOut ); */
	static void GetSystemAddress_Old ( __UDPSOCKET__ s, SystemAddress *systemAddressOut );
/* static void GetSystemAddress ( RakNetSocket *s, SystemAddress *systemAddressOut ); */
	static void GetSystemAddress ( __UDPSOCKET__ s, SystemAddress *systemAddressOut );

/* static void SetSocketLayerOverride(SocketLayerOverride *_slo); */
/* static SocketLayerOverride* GetSocketLayerOverride() {return slo;} */

/* 	static int SendTo_PS3Lobby( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress ); */
/* 	static int SendTo_PSP2( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress ); */
/* 	static int SendTo_360( RakNetSocket *s, const char *data, int length, const char *voiceData, int voiceLength, const SystemAddress &systemAddress ); */
/* 	static int SendTo_PC( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress, const char *file, const long line ); */
/* #if defined(WINDOWS_STORE_RT) */
/* 	static int SendTo_WindowsStore8( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress, const char *file, const long line ); */
/* #endif */

/* 	static void SetDoNotFragment( RakNetSocket* listenSocket, int opt ); */
/* 	static void SetSocketOptions( RakNetSocket* listenSocket, bool blockingSocket, bool setBroadcast); */
	static void SetSocketOptions( __UDPSOCKET__ listenSocket, bool blockingSocket, bool setBroadcast);
	

	/* AF_INET (default). For IPV6, use AF_INET6. To autoselect, use AF_UNSPEC. */
	static bool GetFirstBindableIP(char firstBindable[128], int ipProto);

private:

/* static SocketLayerOverride *slo; */
};

} /* RakNet 命名空间 */
