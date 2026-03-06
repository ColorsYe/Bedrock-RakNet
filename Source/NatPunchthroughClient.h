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
 * 包含客户端 NAT 穿透插件。
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_NatPunchthroughClient==1

#pragma once
#include "RakNetTypes.h"
#include "Export.h"
#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "SocketIncludes.h"
#include "DS_List.h"
#include "RakString.h"
#include "DS_Queue.h"

/* Trendnet TEW-632BRP 有时从端口 1024 开始并顺序递增。 */
/* Zonnet zsr1134we. 回复发出到网络，但总是被远程路由器吸收？？ */
/* Dlink ebr2310 到 Trendnet 正常 */
/* Trendnet TEW-652BRP 到 Trendnet 632BRP 正常 */
/* Trendnet TEW-632BRP 到 Trendnet 632BRP 正常 */
/* Buffalo WHR-HP-G54 正常 */
/* Netgear WGR614 正常 */

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
struct Packet;
#if _RAKNET_SUPPORT_PacketLogger==1
class PacketLogger;
#endif

/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT PunchthroughConfiguration
{
	/*
	 * 内部使用: (15 ms * 2 次尝试 + 30 等待) * 5 端口 * 8 玩家 = 2.4 秒
	 * 外部: (50 ms * 8 次发送 + 200 等待) * 2 端口 * 8 玩家 = 9.6 秒
	 * 总计: 8 秒
	 */
	PunchthroughConfiguration() {
		TIME_BETWEEN_PUNCH_ATTEMPTS_INTERNAL=15;
		TIME_BETWEEN_PUNCH_ATTEMPTS_EXTERNAL=50;
		UDP_SENDS_PER_PORT_INTERNAL=2;
		UDP_SENDS_PER_PORT_EXTERNAL=8;
		INTERNAL_IP_WAIT_AFTER_ATTEMPTS=30;
		MAXIMUM_NUMBER_OF_INTERNAL_IDS_TO_CHECK=5; /* / 设为 0 以不进行局域网连接 */
		MAX_PREDICTIVE_PORT_RANGE=2;
		EXTERNAL_IP_WAIT_BETWEEN_PORTS=200;
		EXTERNAL_IP_WAIT_AFTER_FIRST_TTL=100;
		EXTERNAL_IP_WAIT_AFTER_ALL_ATTEMPTS=EXTERNAL_IP_WAIT_BETWEEN_PORTS;
		retryOnFailure=false;
	}

	/* 每次 UDP 发送之间的时间间隔 */
	RakNet::Time TIME_BETWEEN_PUNCH_ATTEMPTS_INTERNAL;
	RakNet::Time TIME_BETWEEN_PUNCH_ATTEMPTS_EXTERNAL;

	/* 在放弃并转到下一个端口之前，对一个端口的尝试次数 */
	int UDP_SENDS_PER_PORT_INTERNAL;
	int UDP_SENDS_PER_PORT_EXTERNAL;

	/* 放弃一个内部端口后，等待多久再尝试下一个端口 */
	int INTERNAL_IP_WAIT_AFTER_ATTEMPTS;

	/* 在最后已知起始端口之后，尝试多少个外部端口 */
	int MAX_PREDICTIVE_PORT_RANGE;

	/* 发送 TTL 后，等待多久才开始第一次穿透尝试 */
	int EXTERNAL_IP_WAIT_AFTER_FIRST_TTL;

	/* 放弃一个外部端口后，等待多久再尝试下一个端口 */
	int EXTERNAL_IP_WAIT_BETWEEN_PORTS;

	/* 尝试所有外部端口后，等待多久才返回 ID_NAT_PUNCHTHROUGH_FAILED */
	int EXTERNAL_IP_WAIT_AFTER_ALL_ATTEMPTS;

	/*
	 * 尝试连接的最大内部 IP 地址数。
	 * 不能大于 MAXIMUM_NUMBER_OF_INTERNAL_IDS
	 * 应足够高以尝试大多数计算机上的所有内部 IP 地址
	 */
	int MAXIMUM_NUMBER_OF_INTERNAL_IDS_TO_CHECK;

	/*
	 * 若第一次穿透尝试失败，则再试一次
	 * 这有时会成功，因为远程路由器在响应来自另一系统的较低编号端口之前，先等待较高编号端口上的传入消息
	 */
	bool retryOnFailure;
};

/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT NatPunchthroughDebugInterface
{
	NatPunchthroughDebugInterface() {}
	virtual ~NatPunchthroughDebugInterface() {}
	virtual void OnClientMessage(const char *msg)=0;
};

/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT NatPunchthroughDebugInterface_Printf : public NatPunchthroughDebugInterface
{
	virtual void OnClientMessage(const char *msg);
};

#if _RAKNET_SUPPORT_PacketLogger==1
/* \ingroup NAT_PUNCHTHROUGH_GROUP */
struct RAK_DLL_EXPORT NatPunchthroughDebugInterface_PacketLogger : public NatPunchthroughDebugInterface
{
	/* 设置为非零值以写入数据包日志记录器！ */
	PacketLogger *pl;

	NatPunchthroughDebugInterface_PacketLogger() {pl=0;}
	~NatPunchthroughDebugInterface_PacketLogger() {}
	virtual void OnClientMessage(const char *msg);
};
#endif

/*
 * NAT 穿透的客户端代码
 * 保持与 NatPunchthroughServer 的连接，以处理通过 NatPunchthroughClient 发起的传入连接请求<BR>
 * 客户端将向端口发送数据报以估算下一个端口<BR>
 * 估算端口后，将同时与另一个客户端建立连接。
 * 另见 NatTypeDetectionClient
 * 另见 http://www.jenkinssoftware.com/raknet/manual/natpunchthrough.html
 * \ingroup NAT_PUNCHTHROUGH_GROUP
 */
class RAK_DLL_EXPORT NatPunchthroughClient : public PluginInterface2
{
public:

	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(NatPunchthroughClient)

	NatPunchthroughClient();
	~NatPunchthroughClient() noexcept;

	/*
	 * 若运行 NATPunchthroughServer 的 RakPeer 实例绑定到两个 IP 地址，则可调用 FindRouterPortStride()
	 * 这将确定你的路由器在分配端口时使用的步长（前提是路由器为全锥型）
	 * 此函数也会在调用 OpenNAT 时自动调用——但如果在连接到中介时更早调用，可以加快过程
	 * 参数[输入] destination 要穿透的系统。必须已连接到中介
	 */
	void FindRouterPortStride(const SystemAddress &facilitator);

	/*
	 * 穿透 NAT。不连接，只尝试设置路由表
	 * 参数[输入] destination 要穿透的系统。必须已连接到中介
	 * 参数[输入] facilitator 我们已连接到的运行 NatPunchthroughServer 插件的系统
	 * 另见 OpenNATGroup()
	 * 成功时将收到 ID_NAT_PUNCHTHROUGH_SUCCEEDED
	 * 各种类型的失败将收到 ID_NAT_TARGET_NOT_CONNECTED、ID_NAT_TARGET_UNRESPONSIVE、ID_NAT_CONNECTION_TO_TARGET_LOST、ID_NAT_ALREADY_IN_PROGRESS 或 ID_NAT_PUNCHTHROUGH_FAILED
	 * 但是，如果你失去与中介的连接，可能不一定会收到上述消息
	 */
	bool OpenNAT(RakNetGUID destination, const SystemAddress &facilitator);

	/*
	 * 已废弃 请参阅更灵活的 FullyConnectedMesh2::StartVerifiedJoin()
	 * 与为系统列表调用 OpenNAT 相同，但回复会延迟到所有系统均通过为止。
	 * 这对于点对点游戏非常有用，你希望连接到远程会话中的每个系统，而不仅仅是某个特定系统
	 * 注意: 对于云计算，组中的所有系统必须连接到同一个中介，因为我们只指定一个
	 * 成功时将收到 ID_NAT_GROUP_PUNCH_SUCCEEDED
	 * 各种类型的失败将收到 ID_NAT_TARGET_NOT_CONNECTED、ID_NAT_ALREADY_IN_PROGRESS 或 ID_NAT_GROUP_PUNCH_FAILED
	 * 但是，如果你失去与中介的连接，可能不一定会收到上述消息
	 *
	bool OpenNATGroup(DataStructures::List<RakNetGUID> destinationSystems, const SystemAddress &facilitator);
	*/

	/*
	 * 如需修改系统配置
	 * 穿透进行中时不要修改结构体中的变量
	 */
	PunchthroughConfiguration* GetPunchthroughConfiguration();

	/*
	 * 设置用于接收调试消息的回调
	 * 参数[输入] i 指向接口的指针。该指针会被存储，因此在进行中不要删除它。传入 0 以清除。
	 */
	void SetDebugInterface(NatPunchthroughDebugInterface *i);

	/* 获取应传递给 UPNP 的端口映射（适用于 miniupnpc-1.6.20120410，用于函数 UPNP_AddPortMapping） */
	void GetUPNPPortMappings(char *externalPort, char *internalPort, const SystemAddress &natPunchthroughServerAddress);

	/* 内部使用 用于插件处理 */
	void Update() override;

	/* 内部使用 用于插件处理 */
	PluginReceiveResult OnReceive(Packet *packet) override;

	/* 内部使用 用于插件处理 */
	void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) override;

	/* 内部使用 用于插件处理 */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;

	void OnAttach() override;
	void OnDetach() override;
	void OnRakPeerShutdown() override;
	void Clear();

	struct SendPing
	{
		RakNet::Time nextActionTime;
		SystemAddress targetAddress;
		SystemAddress facilitator;
		SystemAddress internalIds[MAXIMUM_NUMBER_OF_INTERNAL_IDS];
		RakNetGUID targetGuid;
		bool weAreSender;
		int attemptCount;
		int retryCount;
		int punchingFixedPortAttempts; /* 仅用于 TestMode::PUNCHING_FIXED_PORT */
		uint16_t sessionId;
		bool sentTTL;
		/* 优先使用内部 IP 地址，因为如果我们在局域网上，不希望尝试通过互联网连接 */
		enum TestMode
		{
			TESTING_INTERNAL_IPS,
			WAITING_FOR_INTERNAL_IPS_RESPONSE,
			/* SEND_WITH_TTL, */
			TESTING_EXTERNAL_IPS_FACILITATOR_PORT_TO_FACILITATOR_PORT,
			TESTING_EXTERNAL_IPS_1024_TO_FACILITATOR_PORT,
			TESTING_EXTERNAL_IPS_FACILITATOR_PORT_TO_1024,
			TESTING_EXTERNAL_IPS_1024_TO_1024,
			WAITING_AFTER_ALL_ATTEMPTS,

			/* Trendnet 将远程端口重映射到 1024。 */
			/* 如果你继续在同一 IP 的不同端口上穿透，它会封禁你，通信变成单向的 */
			PUNCHING_FIXED_PORT,

			/* 尝试端口 1024-1028 */
		} testMode;
	} sp;

protected:
	unsigned short mostRecentExternalPort;
	/* void OnNatGroupPunchthroughRequest(Packet *packet); */
	void OnFailureNotification(Packet *packet);
	/* void OnNatGroupPunchthroughReply(Packet *packet); */
	void OnGetMostRecentPort(Packet *packet);
	void OnConnectAtTime(Packet *packet);
	unsigned int GetPendingOpenNATIndex(RakNetGUID destination, const SystemAddress &facilitator);
	void SendPunchthrough(RakNetGUID destination, const SystemAddress &facilitator);
	void QueueOpenNAT(RakNetGUID destination, const SystemAddress &facilitator);
	void SendQueuedOpenNAT();
	void SendTTL(const SystemAddress &sa);
	void SendOutOfBand(SystemAddress sa, MessageID oobId);
	void OnPunchthroughFailure();
	void OnReadyForNextPunchthrough();
	void PushFailure();
	bool RemoveFromFailureQueue();
	void PushSuccess();

	PunchthroughConfiguration pc;
	NatPunchthroughDebugInterface *natPunchthroughDebugInterface;

	/* 第一次 NAT 尝试失败时，将其添加到 failedAttemptList 并稍后重试，因为有时稍后重试可以解决问题 */
	/* 第二次失败时，返回 ID_NAT_PUNCHTHROUGH_FAILED */
	struct AddrAndGuid
	{
		SystemAddress addr;
		RakNetGUID guid;
	};
	DataStructures::List<AddrAndGuid> failedAttemptList;

	struct DSTAndFac
	{
		RakNetGUID destination;
		SystemAddress facilitator;
	};
	DataStructures::Queue<DSTAndFac> queuedOpenNat;

	void IncrementExternalAttemptCount(RakNet::Time time, RakNet::Time delta);
	unsigned short portStride;
	enum
	{
		HAS_PORT_STRIDE,
		UNKNOWN_PORT_STRIDE,
		CALCULATING_PORT_STRIDE,
		INCAPABLE_PORT_STRIDE
	} hasPortStride;
	RakNet::Time portStrideCalTimeout;

	/*
	struct TimeAndGuid
	{
		RakNet::Time time;
		RakNetGUID guid;
	};
	DataStructures::List<TimeAndGuid> groupRequestsInProgress;

	struct GroupPunchRequest
	{
		SystemAddress facilitator;
		DataStructures::List<RakNetGUID> pendingList;
		DataStructures::List<RakNetGUID> passedListGuid;
		DataStructures::List<SystemAddress> passedListAddress;
		DataStructures::List<RakNetGUID> failedList;
		DataStructures::List<RakNetGUID> ignoredList;
	};
	DataStructures::List<GroupPunchRequest*> groupPunchRequests;
	void UpdateGroupPunchOnNatResult(SystemAddress facilitator, RakNetGUID targetSystem, SystemAddress targetSystemAddress, int result); // 0=失败, 1=成功, 2=忽略
	*/
};

} /* RakNet 命名空间 */

#endif
