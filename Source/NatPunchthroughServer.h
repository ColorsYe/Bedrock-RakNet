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
 * 包含服务器端 NAT 穿透插件。
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_NatPunchthroughServer==1

#pragma once
#include "RakNetTypes.h"
#include "Export.h"
#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "SocketIncludes.h"
#include "DS_OrderedList.h"
#include "RakString.h"

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
struct Packet;
#if _RAKNET_SUPPORT_PacketLogger==1
class PacketLogger;
#endif

/*
 * \defgroup NAT_PUNCHTHROUGH_GROUP NatPunchthrough
 * 使两个均位于路由器后面的系统能够相互连接
 * * \ingroup PLUGINS_GROUP
 */

/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT NatPunchthroughServerDebugInterface
{
	NatPunchthroughServerDebugInterface() {}
	virtual ~NatPunchthroughServerDebugInterface() {}
	virtual void OnServerMessage(const char *msg)=0;
};

/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT NatPunchthroughServerDebugInterface_Printf : public NatPunchthroughServerDebugInterface
{
	virtual void OnServerMessage(const char *msg);
};

#if _RAKNET_SUPPORT_PacketLogger==1
/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT NatPunchthroughServerDebugInterface_PacketLogger : public NatPunchthroughServerDebugInterface
{
	/* 设置为非零值以写入数据包日志记录器！ */
	PacketLogger *pl;

	NatPunchthroughServerDebugInterface_PacketLogger() {pl=0;}
	~NatPunchthroughServerDebugInterface_PacketLogger() {}
	virtual void OnServerMessage(const char *msg);
};
#endif

/*
 * NAT 穿透的服务器端代码
 * 保持与 NatPunchthroughServer 的连接，以处理通过 NatPunchthroughClient 发起的传入连接请求<BR>
 * 服务器维护两个客户端可连接的套接字，以便估算下一个端口选择<BR>
 * 服务器将端口估算值、当前与服务器通信的公共端口，以及开始连接尝试的时间告知另一个客户端
 * 另见 NatTypeDetectionClient
 * 另见 http://www.jenkinssoftware.com/raknet/manual/natpunchthrough.html
 * \ingroup NAT_PUNCHTHROUGH_GROUP
 */
class RAK_DLL_EXPORT NatPunchthroughServer : public PluginInterface2
{
public:

	STATIC_FACTORY_DECLARATIONS(NatPunchthroughServer)

	/* 构造函数 */
	NatPunchthroughServer();

	/* 析构函数 */
	virtual ~NatPunchthroughServer();

	/*
	 * 设置用于接收调试消息的回调
	 * 参数[输入] i 指向接口的指针。该指针会被存储，因此在进行中不要删除它。传入 0 以清除。
	 */
	void SetDebugInterface(NatPunchthroughServerDebugInterface *i);

	/* 内部使用 用于插件处理 */
	void Update() override;

	/* 内部使用 用于插件处理 */
	PluginReceiveResult OnReceive(Packet *packet) override;

	/* 内部使用 用于插件处理 */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) override;

	/* 每个已连接的用户都有一个就绪状态。就绪表示准备好进行 NAT 穿透。 */
	struct User;
	struct ConnectionAttempt
	{
		ConnectionAttempt() {sender=0; recipient=0; startTime=0; attemptPhase=NAT_ATTEMPT_PHASE_NOT_STARTED;}
		User *sender, *recipient;
		uint16_t sessionId;
		RakNet::Time startTime;
		enum
		{
			NAT_ATTEMPT_PHASE_NOT_STARTED,
			NAT_ATTEMPT_PHASE_GETTING_RECENT_PORTS,
		} attemptPhase;
	};
	struct User
	{
		RakNetGUID guid;
		SystemAddress systemAddress;
		unsigned short mostRecentPort;
		bool isReady;
		DataStructures::OrderedList<RakNetGUID,RakNetGUID> groupPunchthroughRequests;

		DataStructures::List<ConnectionAttempt *> connectionAttempts;
		bool HasConnectionAttemptToUser(User *user);
		void DerefConnectionAttempt(ConnectionAttempt *ca);
		void DeleteConnectionAttempt(ConnectionAttempt *ca);
		void LogConnectionAttempts(RakNet::RakString &rs);
	};
	RakNet::Time lastUpdate;
	static int NatPunchthroughUserComp( const RakNetGUID &key, User * const &data );
protected:
	void OnNATPunchthroughRequest(Packet *packet);
	DataStructures::OrderedList<RakNetGUID, User*, NatPunchthroughServer::NatPunchthroughUserComp> users;

	void OnGetMostRecentPort(Packet *packet);
	void OnClientReady(Packet *packet);

	void SendTimestamps();
	void StartPendingPunchthrough();
	void StartPunchthroughForUser(User*user);
	uint16_t sessionId;
	NatPunchthroughServerDebugInterface *natPunchthroughServerDebugInterface;

	SystemAddress boundAddresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS];
	unsigned char boundAddressCount;

};

} /* RakNet 命名空间 */

#endif
