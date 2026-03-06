/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 套接字函数跨平台宏定义（内部使用） */
#pragma once


#if   defined(WINDOWS_STORE_RT)
	#include "WinRTSocketAdapter.h"
	#define accept__ WinRTAccept
	#define connect__ WinRTConnect
	#define closesocket__ WinRTClose
	#define socket__ WinRTCreateDatagramSocket
	#define bind__ WinRTBind
	#define getsockname__ RNS2_WindowsStore8::WinRTGetSockName
	#define getsockopt__ WinRTGetSockOpt
	#define inet_addr__ RNS2_WindowsStore8::WinRTInet_Addr
	#define ioctlsocket__ RNS2_WindowsStore8::WinRTIOCTLSocket
	#define listen__ WinRTListen
	#define recv__ WinRTRecv
	#define recvfrom__ WinRTRecvFrom
	#define select__ WinRTSelect
	#define send__ WinRTSend
	#define sendto__ WinRTSendTo
	#define setsockopt__ RNS2_WindowsStore8::WinRTSetSockOpt
	#define shutdown__ WinRTShutdown
	#define WSASendTo__ WinRTSendTo
#else

	#if   defined(_WIN32)
		#define closesocket__ closesocket
		#define select__ select
	#elif defined(__native_client__)
		/* namespace RakNet { void CloseSocket(SOCKET s); } */
		/* #define closesocket__ RakNet::CloseSocket */
		#define select__ select
	#else
		#define closesocket__ close
		#define select__ select
	#endif
	#define accept__ accept
	#define connect__ connect



		#define socket__ socket

	#define bind__ bind
	#define getsockname__ getsockname
	#define getsockopt__ getsockopt



		#define inet_addr__ inet_addr

	#define ioctlsocket__ ioctlsocket
	#define listen__ listen
	#define recv__ recv
	#define recvfrom__ recvfrom



		#define sendto__ sendto

	#define send__ send



		#define setsockopt__ setsockopt

	#define shutdown__ shutdown
	#define WSASendTo__ WSASendTo
#endif
