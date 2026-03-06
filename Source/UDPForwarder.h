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
 * 转发 UDP 数据报。独立于 RakNet 的协议。
 *
 */



#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_UDPForwarder==1

#pragma once
#include "Export.h"
#include "RakNetTypes.h"
#include "SocketIncludes.h"
#include "UDPProxyCommon.h"
#include "SimpleMutex.h"
#include "RakString.h"
#include "RakThread.h"
#include "DS_Queue.h"
#include "DS_OrderedList.h"
#include "LocklessTypes.h"
#include "DS_ThreadsafeAllocatingQueue.h"

namespace RakNet
{

enum UDPForwarderResult
{
	UDPFORWARDER_FORWARDING_ALREADY_EXISTS,
	UDPFORWARDER_NO_SOCKETS,
	UDPFORWARDER_BIND_FAILED,
	UDPFORWARDER_INVALID_PARAMETERS,
	UDPFORWARDER_NOT_RUNNING,
	UDPFORWARDER_SUCCESS,
	UDPFORWARDER_RESULT_COUNT
};

/*
 * 转发 UDP 数据报。独立于 RakNet 的协议。
 * \ingroup NAT_PUNCHTHROUGH_GROUP
 */
class RAK_DLL_EXPORT UDPForwarder
{
public:
	UDPForwarder();
	virtual ~UDPForwarder();

	/*
	 * 启动 system
	 * 必须 to call before StartForwarding
	 */
	void Startup();

	/* Stops the system, and frees all sockets */
	void Shutdown();

	/*
	 * 设置 maximum number of forwarding entries allowed
	 * 将according设置为your available bandwidth and the estimated average bandwidth per forwarded address
	 * 参数[输入] maxEntries The maximum number of simultaneous forwarding entries. Defaults to 64 (32 connections)
	 */
	void SetMaxForwardEntries(unsigned short maxEntries);

	/* 返回值: The maxEntries parameter passed to SetMaxForwardEntries(), or the default if it was never called */
	int GetMaxForwardEntries(void) const;

	/* 返回值: How many entries have been used */
	int GetUsedForwardEntries(void) const;

	/*
	 * Forwards datagrams from source to destination, and vice-versa
	 * Does nothing if this forward entry already exists via a previous call
	 * 前提条件: Call Startup()
	 * 注意: RakNet's protocol will ensure a message is sent at least every 15 seconds, so if routing RakNet messages, it is a reasonable value for timeoutOnNoDataMS, plus an some extra seconds for latency
	 * 参数[输入] source The source IP and port
	 * 参数[输入] destination Where to forward to (and vice-versa)
	 * 参数[输入] timeoutOnNoDataMS If no messages are forwarded for this many MS, then automatically remove this entry.
	 * 参数[输入] forceHostAddress Force binding on a particular address. 0 to use any.
	 * 参数[输入] socketFamily IP version: For IPV4, use AF_INET (default). For IPV6, use AF_INET6. To autoselect, use AF_UNSPEC.
	 * 参数[输出] forwardingPort New opened port for forwarding
	 * 参数[输出] forwardingSocket New opened socket for forwarding
	 * 返回值: UDPForwarderResult
	 */
	UDPForwarderResult StartForwarding(
		SystemAddress source, SystemAddress destination, RakNet::TimeMS timeoutOnNoDataMS,
		const char *forceHostAddress, unsigned short socketFamily,
		unsigned short *forwardingPort, __UDPSOCKET__ *forwardingSocket);

	/*
	 * No longer forward datagrams from source to destination
	 * 参数[输入] source The source IP and port
	 * 参数[输入] destination Where to forward to
	 */
	void StopForwarding(SystemAddress source, SystemAddress destination);


	struct ForwardEntry
	{
		ForwardEntry();
		~ForwardEntry() noexcept;
		SystemAddress addr1Unconfirmed, addr2Unconfirmed, addr1Confirmed, addr2Confirmed;
		RakNet::TimeMS timeLastDatagramForwarded;
		__UDPSOCKET__ socket;
		RakNet::TimeMS timeoutOnNoDataMS;
		short socketFamily;
	};


protected:
	friend RAK_THREAD_DECLARATION(UpdateUDPForwarderGlobal);

	void UpdateUDPForwarder();
	void RecvFrom(RakNet::TimeMS curTime, ForwardEntry *forwardEntry);

	struct StartForwardingInputStruct
	{
		SystemAddress source;
		SystemAddress destination;
		RakNet::TimeMS timeoutOnNoDataMS;
		RakString forceHostAddress;
		unsigned short socketFamily;
		unsigned int inputId;
	};

	DataStructures::ThreadsafeAllocatingQueue<StartForwardingInputStruct> startForwardingInput;

	struct StartForwardingOutputStruct
	{
		unsigned short forwardingPort;
		__UDPSOCKET__ forwardingSocket;
		UDPForwarderResult result;
		unsigned int inputId;
	};
	DataStructures::Queue<StartForwardingOutputStruct> startForwardingOutput;
	SimpleMutex startForwardingOutputMutex;

	struct StopForwardingStruct
	{
		SystemAddress source;
		SystemAddress destination;
	};
	DataStructures::ThreadsafeAllocatingQueue<StopForwardingStruct> stopForwardingCommands;
	unsigned int nextInputId;

	/* New entries are added to forwardListNotUpdated */
	DataStructures::List<ForwardEntry*> forwardListNotUpdated;
/* SimpleMutex forwardListNotUpdatedMutex; */

	unsigned short maxForwardEntries;
	RakNet::LocklessUint32_t isRunning, threadRunning;

};

} /* 命名空间结束 */

#endif
