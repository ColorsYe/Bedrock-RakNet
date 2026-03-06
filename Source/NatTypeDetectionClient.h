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
 * 包含客户端的 NAT 类型检测代码
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_NatTypeDetectionClient==1

#pragma once
#include "RakNetTypes.h"
#include "Export.h"
#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "SocketIncludes.h"
#include "DS_OrderedList.h"
#include "RakString.h"
#include "NatTypeDetectionCommon.h"

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
struct Packet;

	/*
	 * Client code for NatTypeDetection
	 * 参见 NatTypeDetectionServer.h for algorithm
	 * To use, just connect to the server, and call DetectNAT
	 * You will get back ID_NAT_TYPE_DETECTION_RESULT with one of the enumerated values of NATTypeDetectionResult found in NATTypeDetectionCommon.h
	 * See also http://www.jenkinssoftware.com/raknet/manual/natpunchthrough.html
	 * 另见 NatPunchthroughClient
	 * 另见 NatTypeDetectionServer
	 * \ingroup NAT_TYPE_DETECTION_GROUP
	 */
	class RAK_DLL_EXPORT NatTypeDetectionClient : public PluginInterface2, public RNS2EventHandler
	{
	public:

		/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
		STATIC_FACTORY_DECLARATIONS(NatTypeDetectionClient)

		/* 构造函数 */
		NatTypeDetectionClient();

		/* 析构函数 */
		virtual ~NatTypeDetectionClient();

		/*
		 * Send the message to the server to detect the nat type
		 * Server must be running NatTypeDetectionServer
		 * We must already be connected to the server
		 * 参数[输入] serverAddress address of the server
		 */
		void DetectNATType(SystemAddress _serverAddress);

		/* 内部使用 For plugin handling */
		void Update() override;

		/* 内部使用 For plugin handling */
		PluginReceiveResult OnReceive(Packet *packet) override;

		void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
		void OnRakPeerShutdown() override;
		void OnDetach() override;

		virtual void OnRNS2Recv(RNS2RecvStruct *recvStruct);
		virtual void DeallocRNS2RecvStruct(RNS2RecvStruct *s, const char *file, unsigned int line);
		virtual RNS2RecvStruct *AllocRNS2RecvStruct(const char *file, unsigned int line);
	protected:
		DataStructures::Queue<RNS2RecvStruct*> bufferedPackets;
		SimpleMutex bufferedPacketsMutex;
		
		RakNetSocket2* c2;
		/* unsigned short c2Port; */
		void Shutdown();
		void OnCompletion(NATTypeDetectionResult result);
		bool IsInProgress(void) const;

		void OnTestPortRestricted(Packet *packet);
		SystemAddress serverAddress;
	};


}


#endif
