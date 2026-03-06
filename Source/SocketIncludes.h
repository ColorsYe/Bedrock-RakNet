#pragma once
/* 此文件仅为包含 SOCKET 类型而存在的各种平台头文件 */

#ifdef __native_client__
#define _PP_Instance_ PP_Instance
#else
#define _PP_Instance_ int
#endif





















#if   defined(WINDOWS_STORE_RT)
	#include <windows.h>
	#include "WinRTSockAddr.h"
	using __UDPSOCKET__ = Windows::Networking::Sockets::DatagramSocket^;
	using __TCPSOCKET__ = Windows::Networking::Sockets::StreamSocket^;
	using socklen_t = unsigned int;
	#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0
	#define FIONBIO 0
	#define LocalFree(x)
	/* using Windows.Networking; */
	/* using Windows.Networking.Sockets; */
	/* 参见 http://msdn.microsoft.com/en-us/library/windows/apps/windows.networking.sockets.datagramsocketcontrol */
#elif defined(_WIN32)
	/* IP_DONTFRAGMENT 在 winsock 1 和 winsock 2 之间不同。因此 Winsock2.h 必须链接 Ws2_32.lib */
	/* winsock.h 必须链接 WSock32.lib。如果混淆这两者，标志将无法正确工作 */
	/* WinRT: http://msdn.microsoft.com/en-us/library/windows/apps/windows.networking.sockets */
	/* 示例代码: http://stackoverflow.com/questions/10290945/correct-use-of-udp-datagramsocket */
	#include <winsock2.h>
	using __UDPSOCKET__ = SOCKET;
	using __TCPSOCKET__ = SOCKET;
	using socklen_t = int;
#else
	#define closesocket close
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>

	#ifdef __native_client__
		#include "ppapi/cpp/private/net_address_private.h"
		#include "ppapi/c/pp_bool.h"
		#include "ppapi/c/pp_errors.h"
		#include "ppapi/cpp/completion_callback.h"
		#include "ppapi/cpp/instance_handle.h"
		#include "ppapi/cpp/module.h"
		#include "ppapi/cpp/module_impl.h"
		#include "ppapi/c/pp_errors.h"
		#include "ppapi/c/pp_module.h"
		#include "ppapi/c/pp_var.h"
		#include "ppapi/c/pp_resource.h"
		#include "ppapi/c/ppb.h"
		#include "ppapi/c/ppb_instance.h"
		#include "ppapi/c/ppb_messaging.h"
		#include "ppapi/c/ppb_var.h"
		#include "ppapi/c/ppp.h"
		#include "ppapi/c/ppb_core.h"
		#include "ppapi/c/ppp_instance.h"
		#include "ppapi/c/ppp_messaging.h"
		#include "ppapi/c/pp_input_event.h"
		#include "ppapi/c/pp_completion_callback.h"
		/* UDP 专用 - 'private' 文件夹复制自 chromium src/ppapi/c 头文件目录 */
		#include "ppapi/c/private/ppb_udp_socket_private.h"
		#include "ppapi/cpp/private/net_address_private.h"
		using __UDPSOCKET__ = PP_Resource;
		using __TCPSOCKET__ = PP_Resource;
	#else
		/* #include "RakMemoryOverride.h" */
		/* Unix/Linux 使用 int 作为套接字类型 */
		using __UDPSOCKET__ = int;
		using __TCPSOCKET__ = int;
#endif

#endif
