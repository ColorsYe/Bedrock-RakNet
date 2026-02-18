#pragma once
// All this crap just to include type SOCKET

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
	// using Windows.Networking;
	// using Windows.Networking.Sockets;
	// See http://msdn.microsoft.com/en-us/library/windows/apps/windows.networking.sockets.datagramsocketcontrol
#elif defined(_WIN32)
	// IP_DONTFRAGMENT is different between winsock 1 and winsock 2.  Therefore, Winsock2.h must be linked againt Ws2_32.lib
	// winsock.h must be linked against WSock32.lib.  If these two are mixed up the flag won't work correctly
	// WinRT: http://msdn.microsoft.com/en-us/library/windows/apps/windows.networking.sockets
	// Sample code: http://stackoverflow.com/questions/10290945/correct-use-of-udp-datagramsocket
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
		//UDP specific - the 'private' folder was copied from the chromium src/ppapi/c headers folder
		#include "ppapi/c/private/ppb_udp_socket_private.h"
		#include "ppapi/cpp/private/net_address_private.h"
		using __UDPSOCKET__ = PP_Resource;
		using __TCPSOCKET__ = PP_Resource;
	#else
		//#include "RakMemoryOverride.h"
		/// Unix/Linux uses ints for sockets
		using __UDPSOCKET__ = int;
		using __TCPSOCKET__ = int;
#endif

#endif
