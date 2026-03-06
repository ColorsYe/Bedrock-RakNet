/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

#pragma once

/* Windows 平台头文件包含 */
#if   defined (WINDOWS_STORE_RT)
#include <windows.h>
#include <winsock.h>
#elif defined (_WIN32)
#include <WinSock2.h>
#include <windows.h>
#include <Ws2tcpip.h>

/* 必须始终在 windows.h 之前包含 Winsock2.h */
/* 否则会出现以下错误: */
/* winsock2.h(99) : error C2011: 'fd_set' : 'struct' 类型重定义 */
/* winsock2.h(134) : warning C4005: 'FD_SET' : 宏重定义 */
/* winsock.h(83) : 参见 'FD_SET' 的前一个定义 */
/* winsock2.h(143) : error C2011: 'timeval' : 'struct' 类型重定义 */
/* winsock2.h(199) : error C2011: 'hostent' : 'struct' 类型重定义 */
/* winsock2.h(212) : error C2011: 'netent' : 'struct' 类型重定义 */
/* winsock2.h(219) : error C2011: 'servent' : 'struct' 类型重定义 */

#endif
